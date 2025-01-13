// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/vm.h>

#include <orbit/liftoff/scanner/scanner.h>
#include <orbit/liftoff/parser/parser.h>

#include <orbit/liftoff/ir/irbuilder.h>
#include <orbit/liftoff/ir/linearscan.h>

#include <orbit/liftoff/codegen.h>

#include <orbit/liftoff/compiler.h>

using namespace liftoff;
using namespace liftoff::ir;
using namespace liftoff::parser;

orbiter::datatype::HList Compiler::BuildCodesList(IRContext *ir) {
    orbiter::datatype::HList codes{};

    const auto count = ir->GetSubcontextCount();
    if (count > 0) {
        codes = orbiter::datatype::ListNew(this->isolate_, count);
        if (!codes)
            return {};

        for (int i = 0; i < count; i++) {
            if (!ListAppend(codes.get(), (orbiter::datatype::OObject *) this->Compile(ir->GetSubContext(i)).get()))
                return {};
        }
    }

    return codes;
}

orbiter::datatype::HCode Compiler::Compile(IRContext *ir) {
    LinearScan rAllocator(orbiter::kGeneralPurposeRegistersCount);
    Codegen codegen(this->isolate_);

    rAllocator.Allocate(ir->ComputeLiveIntervals());

    auto code = codegen.Generate(ir);
    if (code) {
        if (ir->GetSubcontextCount() > 0) {
            code->codes = this->BuildCodesList(ir).get();
            if (code->codes == nullptr)
                return {};
        }
    }

    return code;
}

orbiter::datatype::HCode Compiler::Compile(const char *filename, scanner::Scanner &scanner) {
    Parser parser(this->isolate_, filename, scanner);
    IRBuilder builder(this->isolate_, this->level_);

    auto ast = parser.Parse();
    if (!ast) {
        assert(false);
    }

    auto ir = builder.Generate(ast);
    if (ir == nullptr)
        assert(false);

    return this->Compile(ir);
}

orbiter::datatype::HCode Compiler::Compile(const char *filename, FILE *fd) {
    scanner::Scanner scanner(this->isolate_, fd, nullptr, nullptr);

    return this->Compile(filename, scanner);
}
