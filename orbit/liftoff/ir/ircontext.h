// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_IRCONTEXT_H_
#define ORBIT_LIFTOFF_IR_IRCONTEXT_H_

#include <orbit/datatype.h>

#include <orbit/orbiter/datatype/list.h>

#include <orbit/liftoff/ir/basicblock.h>
#include <orbit/liftoff/ir/jblock.h>

namespace liftoff::ir {
    enum class IRContextType {
        FUNCTION,
        METHOD,
        MODULE
    };

    class IRContext {
        Object *objs_ = nullptr;

        orbiter::datatype::HList static_values;

        U32 logical_counter_ = 0;

        friend class Builder;

    public:
        BasicBlock *entry_ = nullptr;
        BasicBlock *current_ = nullptr;

        JBlock *j_chain = nullptr;

        IRContextType type_;

        explicit IRContext(const IRContextType type) noexcept: type_(type) {
        }

        I32 GetIncRVirtCounter() noexcept {
            return (I32) this->logical_counter_++;
        }

        U16 PushStaticValue(orbiter::Isolate *isolate, orbiter::datatype::OObject *value) {
            if (!this->static_values) {
                this->static_values = orbiter::datatype::ListNew(isolate);
                if (!this->static_values)
                    throw std::bad_alloc();
            }

            if (!ListAppend(this->static_values.get(), value))
                throw std::bad_alloc();

            return this->static_values->length - 1;
        }
    };
}

#endif // !ORBIT_LIFTOFF_IR_IRCONTEXT_H_
