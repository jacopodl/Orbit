// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/datatype/module.h>
#include <orbit/orbiter/datatype/number.h>
#include <orbit/orbiter/datatype/oobject.h>
#include <orbit/orbiter/datatype/orstring.h>

#include <orbit/orbiter/module/modules.h>

using namespace orbiter::datatype;
using namespace orbiter::module;

// *********************************************************************************************************************
// INTERNAL
// *********************************************************************************************************************

static bool SetEndian(orbiter::Isolate *isolate, const TypeInfo *type) {
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    constexpr auto kEndianName = "big";
#else
    constexpr auto kEndianName = "little";
#endif

    auto *prop = TIFindLocalProperty(type, "ENDIAN");
    if (prop == nullptr)
        return false;

    auto endian = ORStringIntern(isolate, kEndianName);
    if (!endian)
        return false;

    prop->value = (OObject *) endian.release();

    return true;
}

static bool SetSizes(orbiter::Isolate *isolate, const TypeInfo *type) {
#define SET_SIZEOF(name, ctype)                                                     \
    do {                                                                            \
        auto *prop = TIFindLocalProperty(type, name);                               \
        if (prop == nullptr) return false;                                          \
                                                                                    \
        auto value = IntNew(isolate, (IntegerUnderlying) sizeof(ctype));            \
        if (!value) return false;                                                   \
                                                                                    \
        prop->value = (OObject *) value.release();                                  \
    } while (0)

    // One constant per NativeType (obase.h), UNIT excepted (void has no size).
    SET_SIZEOF("SIZEOF_BOOL", bool);
    SET_SIZEOF("SIZEOF_BYTE", uint8_t);

    SET_SIZEOF("SIZEOF_I8", int8_t);
    SET_SIZEOF("SIZEOF_I16", int16_t);
    SET_SIZEOF("SIZEOF_I32", int32_t);
    SET_SIZEOF("SIZEOF_I64", int64_t);
    SET_SIZEOF("SIZEOF_ISIZE", intptr_t);

    SET_SIZEOF("SIZEOF_U8", uint8_t);
    SET_SIZEOF("SIZEOF_U16", uint16_t);
    SET_SIZEOF("SIZEOF_U32", uint32_t);
    SET_SIZEOF("SIZEOF_U64", uint64_t);
    SET_SIZEOF("SIZEOF_USIZE", size_t);

    SET_SIZEOF("SIZEOF_PTR", void *);

    SET_SIZEOF("SIZEOF_F32", float);
    SET_SIZEOF("SIZEOF_F64", double);

    return true;
#undef SET_SIZEOF
}

static bool FfiInit(Module *self) {
    auto *isolate = O_GET_ISOLATE(self);
    const auto *type = O_GET_TYPE(self);

    if (!SetSizes(isolate, type))
        return false;

    if (!SetEndian(isolate, type))
        return false;

    return true;
}

// *********************************************************************************************************************
// MODULE TABLE
// *********************************************************************************************************************

const ModuleEntry ffi_entries[] = {
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_BOOL", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_BYTE", nullptr),

    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_I8", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_I16", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_I32", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_I64", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_ISIZE", nullptr),

    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_U8", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_U16", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_U32", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_U64", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_USIZE", nullptr),

    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_PTR", nullptr),

    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_F32", nullptr),
    ORBIT_MODULE_EXPORT_ALIAS("SIZEOF_F64", nullptr),

    ORBIT_MODULE_EXPORT_ALIAS("ENDIAN", nullptr),

    ORBIT_MODULE_SENTINEL
};

ModuleInit ModuleFfi = {
    "::orbit::ffi",
    "@brief Native interop (FFI) platform metadata."
    "\n\n"
    "Exposes the size in bytes of every native type usable in `native` "
    "declarations (SIZEOF_*) and the platform byte order (ENDIAN), so that "
    "buffer layouts for native calls can be computed instead of hardcoded.",
    "1.0.0",
    ffi_entries,
    FfiInit,
    nullptr
};

const ModuleInit *orbiter::module::module_ffi_ = &ModuleFfi;
