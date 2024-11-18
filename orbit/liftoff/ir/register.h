// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_REGISTER_H_
#define ORBIT_LIFTOFF_IR_REGISTER_H_

#include <orbit/liftoff/ir/object.h>

namespace liftoff::ir {
    class Register : public Object {
    public:
        short virtID = -1;
        short physID = -1;

        explicit Register() : Object(ObjectType::REGISTER) {};
    };
}

#endif // !ORBIT_LIFTOFF_IR_REGISTER_H_
