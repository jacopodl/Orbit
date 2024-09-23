// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/context.h>

using namespace orbiter;
using namespace orbiter::datatype;

Context *orbiter::ContextInit() {
#define INIT_TYPE(num, fn)                                      \
    do {                                                        \
        if((ctx->primitive[(int) num] = fn(ctx)) == nullptr)    \
            goto ERROR;                                         \
    } while(0)

#define SETUP_TYPE(num, fn)                             \
    do {                                                \
        if(fn(ctx, ctx->primitive[(int) num]))          \
            goto ERROR;                                 \
    } while(0)

    auto *ctx = (Context *) memory::Alloc(sizeof(Context));
    if (ctx == nullptr)
        return nullptr;


    return ctx;

    ERROR:
    // TODO: Release all type
    return nullptr;
#undef INIT_TYPE
#undef SETUP_TYPE
}
