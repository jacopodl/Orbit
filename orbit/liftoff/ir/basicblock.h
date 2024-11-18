// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_BASICBLOCK_H_
#define ORBIT_LIFTOFF_IR_BASICBLOCK_H_

#include <orbit/liftoff/ir/instruction.h>

namespace liftoff::ir {
    class BasicBlock : Object {
        BasicBlock *next;
        BasicBlock *prev;

        struct {
            Instruction *head;
            Instruction *tail;
        } instr;

        unsigned int offset;
        unsigned int size;
    };
}

#endif // !ORBIT_LIFTOFF_IR_BASICBLOCK_H_
