; Orbit Programming Language - FFI Support
;
; This function is part of Orbit's Foreign Function Interface (FFI) implementation.
; It retrieves a floating-point return value from XMM0 after a native FFI call.
;
; According to the Microsoft x64 ABI:
; - Floating-point return values are placed in XMM0 (double/float)
; - When a native function is invoked through ffi_call (which returns void*),
;   the FP result in XMM0 is invisible to C++. This function must be called
;   immediately after ffi_call, before anything can clobber XMM0, so that the
;   compiler sees the double through the normal ABI return register with no
;   extra moves required.

.code

public fpu_get_return

; Function: fpu_get_return
; Signature: double fpu_get_return()
; Returns: XMM0 - the floating-point return value of the previously called function
fpu_get_return proc
    ret
fpu_get_return endp

end
