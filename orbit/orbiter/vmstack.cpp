// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <cassert>

#include <orbit/orbiter/vmstack.h>

using namespace orbiter;

bool VMStack::Check(Isolate *isolate, MSize current, U32 slots) noexcept {
    auto slots_size = ((MSize) slots) * sizeof(void *);

    if (current + slots_size > this->capacity)
        return this->Grow(isolate, slots);

    return true;
}

bool VMStack::Grow(Isolate *isolate, U32 slots) noexcept {
    auto increment = std::max(((MSize) slots) * sizeof(void *),
                              (this->capacity * kStackGrowthFactor) / kStackGrowthScalingFactor);

    if (this->capacity + increment >= this->limit) {
        increment = ((MSize) slots) * sizeof(void *);

        if (this->capacity + increment >= this->limit)
            return false;
    }

    memory::IsolateAllocator allocator(isolate);
    auto *tmp = allocator.realloc(this->stack, this->capacity + increment);
    if (tmp == nullptr)
        return false;

    this->stack = tmp;
    this->capacity += increment;

    return true;
}

bool VMStack::Init(Isolate *isolate, MSize size, MSize stack_limit) noexcept {
    memory::IsolateAllocator allocator(isolate);

    assert(stack_limit > size);

    this->stack = allocator.alloc<Byte>(size);
    if (this->stack == nullptr)
        return false;

    this->capacity = size;
    this->limit = stack_limit;

    return true;
}
