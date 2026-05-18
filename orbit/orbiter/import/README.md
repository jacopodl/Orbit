# Import machinery

This directory implements Orbit's module import system: resolving an import
string to a concrete module, loading and executing it exactly once, caching the
result, and handling cyclic / concurrent imports without deadlocking the VM.

## Overview

An import is written as a string key:

```orbit
import "a/b/c"          # absolute: resolved from the standard roots
import "./helpers"      # relative: resolved under the importing module
import "::orbit::io"    # builtin: an internal primitive package
```

The string is **not** a filesystem path. It is a logical key. The pipeline is:

```
raw string ──▶ Canonicalize ──▶ canonical Key ──▶ registry lookup
                                                       │
                                       (miss) ─────────┘
                                          │
                                          ▼
                                   locator chain ──▶ Descriptor
                                          │
                                          ▼
                                       loader ──▶ execute top-level
                                          │
                                          ▼
                                   cache + return module
```

The canonical key is **always absolute and OS-independent**, even when the
import was written relative. This is what guarantees a module reached by two
different spellings is loaded only once.

## Import string syntax

Separator is `/`. There is no `.`-form. Path normalization happens during
canonicalization:

- `\` is converted to `/` (Windows paths normalize to unix-style).
- `//` runs are collapsed; `.` segments are collapsed.
- A leading `./` makes the import **relative to the importing module's
  directory** (`dirname(spec.name)`), then folded back into an absolute key.
- `..` is **not supported** and is a hard `ImportError`. Imports are not disk
  paths — you cannot walk upward. Everything is rooted.

### Absolute vs relative vs builtin

| Form              | Base for resolution                | Purpose                                   |
|-------------------|------------------------------------|-------------------------------------------|
| `"x"`             | standard roots                     | stdlib, shared / cross-package code       |
| `"./x"`           | `dirname` of the importing module  | a package's own strictly-local submodules |
| `"::orbit::x"`    | builtin namespace (opaque)         | internal VM primitives (stdlib internals) |

Consequence of dropping `..`: sharing code **laterally or upward inside a
package** must use an absolute import. `./` is for descending into a package's
own submodules only. Its real value is **relocatability** — a package does not
know its own absolute mount name, so `./sub` lets it self-reference even if the
package is mounted under a different root or renamed.

### The `::orbit::<pkg>` namespace

`::` introduces an opaque *scheme*. A `::`-prefixed string bypasses the
filesystem pipeline entirely: no separator conversion, no `.`/`./` handling, no
disk lookup. It is already canonical and is matched verbatim. `:` is illegal in
a disk key, so the builtin namespace can never collide with a disk module.

Builtins are the **primitives**, not the idiomatic surface. User code generally
never writes `import "::orbit::io"` directly. The standard library is the shell
on top: `import "io"` resolves to a stdlib module which internally does
`from "::orbit::io" import write`. There is no short alias — builtins are
reachable only by their explicit `::orbit::` name.

## The locator chain

Resolution is delegated to an ordered chain of **locators**. Each locator is
tried in turn. A locator returns one of three results — the tri-state is
essential:

| Result      | Meaning                                  | Effect on the chain        |
|-------------|------------------------------------------|----------------------------|
| `NotMine`   | not handled here                         | try the next locator       |
| `Found`     | resolved — here is the `Descriptor`      | stop, hand off to a loader |
| `Error`     | **mine, but broken** (e.g. unreadable)   | stop, propagate the error  |

The `Error` state is what prevents a syntax error or a permission failure from
being masked as "module not found": a locator that recognizes the module but
cannot deliver it must report `Error`, not `NotMine`. Only when **every**
locator returns `NotMine` does the import fail with an `ImportError` listing the
locators that were tried.

### Default chain order

```
[0]      builtin     compiled, PINNED, immutable        ::orbit::*
[1..k]   user         registered at runtime, in order   override layer
[k+1]    fs-source    key.orb → key/key.orb             on-disk source
[k+2]    fs-native    key.so / key.dll  (future)        native modules
```

- **Slot 0 (builtin) is pinned and cannot be removed or shadowed.** The
  standard library relies on `::orbit::*` primitives; if user code could
  hijack them it would subvert the stdlib. Namespace integrity is a security
  property.
- **User locators sit before the filesystem locators**, so they may override
  on-disk resolution (virtual filesystems, bundlers, test mocks). They cannot
  override slot 0.

### `fs-source` resolution order

For a disk key, each standard root is probed as:

1. `key.orb` — a file module.
2. `key/key.orb` — a directory-as-package (`is_package = true`); the directory
   and the same-named `.orb` inside it are equivalent to naming the file.
3. *(future)* `key.so` / `key.dll` — native modules.

## Loaders

A locator only *finds* a module and produces a `Descriptor`; a **loader**
turns the descriptor into a live module. `Descriptor.kind` selects the loader:

- **Builtin** — returns the pre-registered primitive module (one-shot init on
  first use; no file execution).
- **Source** — compiles and executes an on-disk `.orb` file.
- **Native** *(future)* — loads a shared object.
- **Virtual** — produced by a runtime locator (see below). Either compiles an
  in-memory source buffer, or adopts a ready-made `Module` object directly.

## `ImportSpec`

Every loaded module carries an `ImportSpec` (its public `__spec`,
effectively immutable to user code):

| Field        | Meaning                                                       |
|--------------|---------------------------------------------------------------|
| `name`       | the canonical absolute key (e.g. `"a/b/c"` or `"::orbit::io"`) |
| `origin`     | absolute on-disk path of the loaded file, or a builtin marker |
| `loader`     | `LoaderKind`: `builtin` / `source` / `native` / `virtual`     |
| `is_package` | true when resolved via the directory-as-package form          |

The base directory for a module's relative (`./`) imports is just
`dirname(spec.name)` — it is derived, not stored, so there is no second copy to
keep consistent. The mutable **load state** lives in the internal
`ModuleEntry`, never in the user-visible `ImportSpec`.

## Concurrency, cycles and the locking model

The core rule: **no registry lock is ever held while a module's top-level code
runs.** A coarse lock around arbitrary user code execution is the root of every
deadlock and serialization problem here. A single `AsyncRWLock` protects only
the registry map, and is held for microseconds — just to read or insert an
entry.

### `ModuleEntry` state machine

Each entry carries a state: `LOADING → LOADED | FAILED`. The module object is
created and inserted **before** the top-level runs (in `LOADING`), so a partial
module always exists; only its degree of "filled-in-ness" changes.

Lookup flow under the brief registry lock:

- **`LOADED`** → return the module.
- **`LOADING`, owned by the current fiber** → cyclic import within one fiber:
  return the **partially initialized** module. The inconsistent window is
  inherent to cycles; names defined *after* the import point are not yet
  visible. (This is the Python-style pragmatic behavior.)
- **`LOADING`, owned by another fiber** → wait cooperatively on the entry's
  waiter queue (the fiber yields; the OS thread is never blocked), then return
  the module or propagate the error on wake.
- **absent** → insert a `LOADING` entry owning the current fiber, release the
  lock, then locate + load + execute **with no lock held**. Nested imports
  recurse into the same flow — no deadlock, because the lock is not held during
  execution. On completion, re-acquire the lock, set `LOADED` / `FAILED`, wake
  waiters, release.

### Failed init

If a module's top-level raises, the entry is **removed** from the cache and
waiters are woken **with the error propagated** (never left hanging). A later
import may retry from scratch. A half-initialized "poisoned" entry must never
linger in the cache. Ordering: mark/remove under the lock, then wake waiters
outside the lock.

### Cross-fiber cyclic deadlock

The state machine alone does not solve the concurrent case: fiber X loads A
(owns it), A imports B; concurrently fiber Y loads B (owns it), B imports A.
X waits on B (owned by Y), Y waits on A (owned by X) — a genuine deadlock.

A **wait-for graph** is maintained (`fiber → module it is blocked on`,
`module → owning fiber`). Before adding a wait edge, the graph is checked for a
cycle; closing a cycle raises an `ImportError` describing the circular import
instead of hanging forever. A hang here is a bug report nobody can ever
diagnose; the explicit error is mandatory, not optional.

## Runtime-extensible locators

The locator chain is extensible from Orbit code, exposed via the
`::orbit::importlib` builtin (the stdlib `import "importlib"` wraps it):

```orbit
importlib.register_locator(fn, prepend = false) -> handle
importlib.unregister_locator(handle)
```

User locators are inserted in the override layer (after slot 0, before the
filesystem locators); `prepend` controls ordering among user locators.

The tri-state maps naturally onto the Orbit callable's outcome:

| `fn(key: str)` returns / does | Locator result | Loader handling                         |
|-------------------------------|----------------|------------------------------------------|
| `nil`                         | `NotMine`      | try the next locator                     |
| raises an exception           | `Error`        | stop, propagate                          |
| a `ModuleSource{name, origin, source}` | `Found` | `Virtual` loader compiles `source`      |
| a ready-made `Module` object  | `Found`        | registry caches it as-is, no execution   |

The two `Found` forms cover both real cases: a locator that *generates* code
(return the source) and one that *fabricates* the module by hand or pulls it
from an external cache (return the `Module`). Internally both are a
`Descriptor` with `kind = Virtual`, distinguished by `module != null` vs
`source != null`. A single C-side bridge `Locator` iterates the registered
Orbit callables and translates return-value / exception into the
`LocateResult` tri-state, so the C chain neither knows nor cares that Orbit
functions sit behind it.

## File layout (planned)

- `importspec.h` — `ImportSpec`, `Descriptor`, `Locator`, the `LoaderKind` /
  `LocateResult` enums.
- the module registry — the `ModuleEntry` state machine, the brief-lock
  protocol, the wait-for graph, the locator chain and the loaders.
