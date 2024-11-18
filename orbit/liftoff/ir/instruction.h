// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_INSTRUCTION_H_
#define ORBIT_LIFTOFF_IR_INSTRUCTION_H_

#include <orbit/orbiter/opcode.h>

#include <orbit/liftoff/ir/object.h>
#include <orbit/liftoff/ir/register.h>

namespace liftoff::ir {
    class Instruction : public Object {
    public:
        orbiter::OPCode opcode;

        explicit Instruction(orbiter::OPCode opcode) : Object(ObjectType::INSTRUCTION), opcode(opcode) {};
    };

    class DefInstruction : public Instruction {
    public:
        Register dest;

        explicit DefInstruction(orbiter::OPCode opcode) : Instruction(opcode) {};
    };

    class BinaryOpInstr : public DefInstruction {
    public:
        Object *left = nullptr;
        Object *right = nullptr;

        explicit BinaryOpInstr(orbiter::OPCode opcode) : DefInstruction(opcode) {};
    };

    class StackLoadInstr : public DefInstruction {
    public:
        unsigned short offset = 0;

        explicit StackLoadInstr() : DefInstruction(orbiter::OPCode::SKLDR) {};
    };
}

#endif // !ORBIT_LIFTOFF_IR_INSTRUCTION_H_
