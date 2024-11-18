// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_OPCODE_H_
#define ORBIT_ORBITER_OPCODE_H_

namespace orbiter {
    // Single instruction format:
    // XXXXXXXX - XXXXXXXX - XXXXXXXX - XXXXXXXX <-- 32Bit
    //  OPCODE  - DEST|SRC -        FLAGS
    //  OPCODE  - DEST|           IMMEDIATE

    enum class OPCode {
        // Load/store
        LDCST = 1,  // Load constr from Code object:    OPCODE | 4 DST | 4 RESERVED  | 16 IMM
        LDIMM,      // Load immediate into register:    OPCODE | 4 DST | 4 SHIFT     | 16 IMM
        MOV,        // Copy value between registers:    OPCODE | 4 DST | 4 SRC       | 16 RESERVED
        MOWN,       // Move value between registers:    OPCODE | 4 DST | 4 SRC       | 16 RESERVED (Move ownership)
        SKLDR       // Load from stack into register:   OPCODE | 4 DST | 4 RESERVED  | 16 OFFSET
    };
}

#endif // !ORBIT_ORBITER_OPCODE_H_
