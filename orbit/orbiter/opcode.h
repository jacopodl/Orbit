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
        LDCST = 1,  // OPCODE | 4 DST | 4 RESERVED  | 16 IMM
        LDIMM,      // OPCODE | 4 DST | 4 SHIFT     | 16 IMM
        MOV,        // OPCODE | 4 DST | 4 SRC       | 16 RESERVED
        MOWN        // OPCODE | 4 DST | 4 SRC       | 16 RESERVED (Move ownership)
    };
}

#endif // !ORBIT_ORBITER_OPCODE_H_
