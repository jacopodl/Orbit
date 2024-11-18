// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_OBJECT_H_
#define ORBIT_LIFTOFF_IR_OBJECT_H_

#include "orbit/orbiter/opcode.h"

namespace liftoff::ir {
    enum class ObjectType {
        INSTRUCTION,
        REGISTER
    };

    class Object {
    protected:
        const ObjectType objType_;

        explicit Object(ObjectType type) : objType_(type) {};
    };
}

#endif // !ORBIT_LIFTOFF_IR_OBJECT_H_
