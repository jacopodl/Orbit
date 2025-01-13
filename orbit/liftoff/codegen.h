// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_CODEGEN_H_
#define ORBIT_LIFTOFF_CODEGEN_H_

#include <orbit/liftoff/ir/ircontext.h>

#include <orbit/orbiter/datatype/code.h>

namespace liftoff {
    class Codegen {
        orbiter::IsolateAllocator allocator_;

        unsigned char *EmitOpcodes(ir::BasicBlock *block, unsigned char *m_code);

    public:
        explicit Codegen(orbiter::Isolate *isolate) noexcept : allocator_(isolate) {
        }

        orbiter::datatype::HCode Generate(ir::IRContext *ir);
    };
}

#endif // |ORBIT_LIFTOFF_CODEGEN_H_
