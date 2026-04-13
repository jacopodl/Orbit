// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/datatype/code.h>

using namespace orbiter::datatype;

bool CodeDtor(const Code *self) {
    const orbiter::memory::IsolateAllocator allocator(O_GET_ISOLATE(self));

    // Cleanup table
    allocator.free(self->cleanup.entries);

    // Exported symbols
    for (auto i = 0; i < self->exported.length; i++)
        O_FAST_DECREF(self->exported.symbols[i].name);

    allocator.free(self->exported.symbols);

    // Native binding
    for (auto i = 0; i < self->native.length; i++) {
        auto binding = self->native.bindings[i];

        O_FAST_DECREF(binding.name);
        O_FAST_DECREF(binding.symbol);
        O_FAST_DECREF(binding.library);

        for (auto j = 0; j < binding.params.count; j++)
            O_FAST_DECREF(binding.params.params[j].name);
    }

    allocator.free(self->native.bindings);

    O_DECREF(self->codes);

    O_FAST_DECREF(self->static_resources);
    O_FAST_DECREF(self->unknown_symbols);

    O_DECREF(self->name);
    O_DECREF(self->doc);

    allocator.free((void *) self->m_code);

    return true;
}

bool orbiter::datatype::CodeTypeSetup(TypeInfo *self) {
    self->dtor = (DtorFn) CodeDtor;

    return true;
}

HCode orbiter::datatype::CodeNew(Isolate *isolate, const unsigned char *m_code, List *unknown_symbols,
                                 List *static_resources, const U32 m_size, const U16 slots_count,
                                 const U16 stack_size) {
    auto *code = MakeObject<Code>(isolate, InstanceType::CODE);
    if (code != nullptr) {
        memory::MemoryZero(((unsigned char *) code) + sizeof(OObject), sizeof(Code) - sizeof(OObject));

        code->static_resources = O_FAST_INCREF(static_resources);
        code->unknown_symbols = O_FAST_INCREF(unknown_symbols);

        code->m_code = m_code;
        code->m_end = m_code + m_size;

        code->slots_count = slots_count;
        code->stack_size = stack_size;
    }

    O_GC_TRACK_RETURN(isolate, code, false);
}

HOType orbiter::datatype::CodeTypeInit(Isolate *isolate) {
    auto code = MakeType(isolate, "Code", InstanceType::CODE, sizeof(Code) - sizeof(OObject), 0, 0);
    return code;
}
