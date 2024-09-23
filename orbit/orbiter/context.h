// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_CONTEXT_H_
#define ORBIT_ORBITER_CONTEXT_H_

#include <orbit/orbiter/datatype/obase.h>

namespace orbiter {
    struct Context {
        datatype::TypeInfo *primitive[datatype::kInstanceTypeCount];
    };

    Context *ContextInit();
}

#endif // !ORBIT_ORBITER_CONTEXT_H_
