# ctbuilder — bug report

**Component:** `orbit/orbiter/datatype/ctbuilder.cpp` (class type construction, instance blueprint, magic-method hook dispatch) · **ID prefix:** `CTB`
**PoCs:** [`poc/ctbuilder/`](poc/ctbuilder/) · **Last reviewed:** 2026-07-17
**Status:** OPEN · PARTIAL · FIXED · WONTFIX — see [GUIDE.md](GUIDE.md).

---

## CTB-001 — Hook dispatchers crash and misname when the receiver is a type object
**Severity:** High (segfault) · **Status:** FIXED (2026-07-17) · **Location:** `orbit/orbiter/datatype/ctbuilder.cpp` (`ClassToString`, `ClassToRepr`, `ClassEqual`, `ClassCompare`, `ClassHash`)

**Fix verified (`ortest/oop/04_type_object_receiver.orb` — 10 checks green; full
suites `ortest` 10/10, tracker PoCs 21/22 with only the open IR-005 red).**

`ops.compare`, `ops.to_string` and `ops.to_repr` are installed on *every* class by
`ClassTypeNew`, and a **type object** resolves its ops through its **superclass**.
So `SomeDerivedClass.tostr()` reaches `ClassToString` with the type object — not
an instance — as receiver. Both assumptions the dispatchers made were wrong there:

```cpp
const auto *type  = O_GET_TYPE(self);                    // superclass, not the type itself
const auto *cache = (ClassBlueprint *) type->aux.data;   // null until first instantiation
if (cache->str != nullptr)                               // <-- null deref
```

Two defects, one root:

1. **Segfault.** The blueprint is built lazily by `InitObjectBlueprint`, called
   from `ClassNew`. A class that was never constructed has `aux.data == nullptr`,
   so the dereference crashed. Minimal reproducer — no hooks needed, just a
   subclass whose parent was never instantiated:
   ```orb
   class Base { pub init() { } }
   class D : Base { pub init() { super.init() } }
   D.tostr()      # SIGSEGV
   ```
   Instantiating `Base` first made it disappear, which confirmed the lazy cache as
   the cause.
2. **Wrong name.** Because `type` was the superclass, the fallback printed the
   parent's name: `D.tostr()` → `<Base at 0x...>` instead of `<D at 0x...>`.

**Fix.** A shared `ClassHookCache(self, &type)` helper resolves both correctly: an
instance yields `(O_GET_TYPE(self), its blueprint)`; a **type object** yields
`(self, nullptr)` — the type names itself, and "no cache" means no instance hook
runs against a non-instance receiver. All five dispatchers now go through it and
treat a null cache as "no hook": identity for `equal`, "not comparable" for
`compare` (so `Compare` raises `NotImplementedError`), the identity hash for
`hash` (matching `Hash()`'s own fallback), and the `<TypeName at 0xADDR>` form for
`to_string` / `to_repr`.

**Note.** `ClassHash` was previously reachable only when a hook existed, but
`ops.hash` on a type object resolves through the superclass, so a parent with a
`hash` hook exposed the same null cache — covered by the same guard.

**PoC:** [`poc/ctbuilder/ctb-001-type-object-receiver.orb`](poc/ctbuilder/ctb-001-type-object-receiver.orb)
Permanent regression coverage lives in `ortest/oop/04_type_object_receiver.orb`.

---
