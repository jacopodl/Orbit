// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/orbiter/datatype/error.h>
#include <orbit/orbiter/datatype/errors.h>
#include <orbit/orbiter/datatype/nativefunc.h>

#ifdef _ORBIT_PLATFORM_WINDOWS

#define __FFI_INTERNAL

#include <orbit/orbiter/native/ffi.h>
#include <orbit/orbiter/native/ffi_internal.h>

using namespace orbiter::native;
using namespace orbiter::datatype;

extern "C" {
void *ffi_call(void *func, const ParamInfo *args, U16 argc, bool stack_only);

double fpu_get_return();
}

bool orbiter::native::CallFunction(Isolate *isolate, HOObject &out, const NativeFunc *func, OObject **args,
                                   const U16 argc) {
    ParamInfo f_args[kMaxSupportedArity]{};

    const auto arity = func->arity;

    if (arity > kMaxSupportedArity) {
        ErrorSet(isolate,
                 FFIError::Details[FFIError::Reason::ID],
                 nullptr,
                 FFIError::Details[FFIError::Reason::INVALID_ARITY],
                 kMaxSupportedArity
        );

        return false;
    }

    if (!PrepareCall(isolate, func, f_args, nullptr, nullptr, args, argc))
        return false;

    void *result = ffi_call(func->handle, f_args, arity, false);

    if (func->ret_type == NativeType::F32)
        *((float *) (&result)) = (float) fpu_get_return();
    else if (func->ret_type == NativeType::F64)
        *((double *) (&result)) = fpu_get_return();

    return true;
}

#endif
