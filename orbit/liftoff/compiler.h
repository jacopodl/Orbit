// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_LIFTOFF_COMPILER_H_
#define ORBIT_LIFTOFF_COMPILER_H_

#include <orbit/liftoff/olevel.h>
#include <orbit/liftoff/ir/ircontext.h>

#include <orbit/orbiter/datatype/code.h>

namespace liftoff {
    class Compiler {
        orbiter::Isolate *isolate_;
        OptimizationLevel level_;

        orbiter::datatype::HList BuildCodesList(ir::IRContext *ir);

        orbiter::datatype::HCode Compile(ir::IRContext *ir);

    public:
        explicit Compiler(orbiter::Isolate *isolate,
                          OptimizationLevel level) noexcept: isolate_(isolate), level_(level) {
        }

        orbiter::datatype::HCode Compile(const char *filename, scanner::Scanner &scanner);

        orbiter::datatype::HCode Compile(const char *filename, FILE *fd);
    };
}

#endif // !ORBIT_LIFTOFF_COMPILER_H_
