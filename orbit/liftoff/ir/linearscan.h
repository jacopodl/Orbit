// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_IR_LINEARSCAN_H_
#define ORBIT_LIFTOFF_IR_LINEARSCAN_H_

#include <vector>

#include <orbit/liftoff/ir/builder.h>
#include <orbit/liftoff/ir/ircontext.h>

namespace liftoff::ir {
    class LinearScan {
        std::vector<LiveInterval *> active_;

        std::vector<U16> free_registers_;
        std::vector<U16> free_stack_slot_;

        Builder builder_;

        U16 stack_offset_ = 0;

        const U16 total_regs_;

        U16 GetFreeStackSlot();

        void EmitStackLoad(Instruction *instruction);

        void ExpireOldIntervals(U32 position);

        void HandleSpill(LiveInterval *interval);

    public:
        explicit LinearScan(orbiter::Isolate *isolate, U16 total_regs);

        void Allocate(IRContext *ir);
    };
}

#endif // !ORBIT_LIFTOFF_IR_LINEARSCAN_H_
