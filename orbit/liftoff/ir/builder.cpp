// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/liftoff/ir/builder.h>

using namespace liftoff::ir;
using namespace orbiter;

Object *Builder::CreateBinaryOp(orbiter::OPCode opcode, Object *left, Object *right) {
    auto binOp = this->allocator_.alloc<BinaryOpInstr>(sizeof(BinaryOpInstr));
    if (binOp != nullptr) {
        new(binOp)BinaryOpInstr(opcode);

        binOp->left = left;
        binOp->right = right;
    }

    return binOp;
}

Instruction *Builder::LoadFromStackOffset(unsigned short offset) {
    auto instr = this->allocator_.alloc<StackLoadInstr>(sizeof(StackLoadInstr));
    if (instr != nullptr) {
        new(instr) StackLoadInstr();
        // TODO: virtID!

        instr->offset = offset;
    }

    return instr;
}