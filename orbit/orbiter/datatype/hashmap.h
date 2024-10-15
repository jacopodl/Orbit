// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_DATATYPE_HASHMAP_H_
#define ORBIT_ORBITER_DATATYPE_HASHMAP_H_

#include <orbit/util/hashmap.h>

#include <orbit/orbiter/memory/memory.h>

#include <orbit/orbiter/datatype/oobject.h>

namespace orbiter::datatype {
    using ORHEntry = HEntry<OObject *, OObject *>;
    using ORHMap = HashMap<OObject *,
        OObject *,
        memory::Alloc,
        memory::Realloc,
        memory::Free,
        Equal,
        Hash>;

    inline auto HASH_ERROR = ORHMap::HASH_ERROR;
}

#endif // !ORBIT_ORBITER_DATATYPE_HASHMAP_H_
