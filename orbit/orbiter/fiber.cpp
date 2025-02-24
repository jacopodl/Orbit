// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <cassert>

#include <orbit/orbiter/fiber.h>

using namespace orbiter;

/**
 * @brief A thread-local pointer to the currently active Fiber object.
 *
 * This variable stores the Fiber instance that is specific to the current thread. It allows
 * the thread to access its associated Fiber object, which is used to manage execution context
 * and isolate state for fibers within the application.
 *
 * @note Each thread has its own independent instance of this variable due to its thread-local
 *       storage specifier.
 */
thread_local Fiber *thl_fiber = nullptr;

Fiber *Fiber::Current() noexcept {
    return thl_fiber;
}

void Fiber::SetCurrent(Fiber *fiber) noexcept {
    thl_fiber = fiber;
}

Fiber *Fiber::New(Isolate *isolate, MSize stack_size, MSize stack_limit) noexcept {
    memory::IsolateAllocator allocator(isolate);

    const auto fiber = allocator.alloc<Fiber>(sizeof(Fiber));
    if (fiber != nullptr) {
        if (!fiber->vm.stack.Init(isolate, stack_size, stack_limit)) {
            allocator.free(fiber);
            return nullptr;
        }

        memory::MemoryZero(&fiber->vm.regs, sizeof(Registers));

        fiber->vm.state = VMState::RUNNABLE;

        fiber->error.value_ = nullptr;
        fiber->error.r_value_ = &fiber->error.value_;

        fiber->isolate = isolate;
    }

    return fiber;
}

void Fiber::Delete(Fiber *fiber) noexcept {
    if (fiber == nullptr)
        return;

    // todo
    assert(false);
}
