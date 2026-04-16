; Orbit Programming Language - FFI Support
;
; This function is part of Orbit's Foreign Function Interface (FFI) implementation.
; It dispatches a call to a native C/C++ function by setting up registers and stack
; according to the Microsoft x64 calling convention, based on a ParamInfo array
; prepared by w64call.cpp.
;
; According to the Microsoft x64 ABI:
; - The first 4 integer/pointer arguments are passed in RCX, RDX, R8, R9
; - The first 4 floating-point arguments are passed in XMM0, XMM1, XMM2, XMM3
; - Integer and FP registers shadow each other positionally (param 0 is either
;   RCX or XMM0, not both), so fp_reg is checked per-slot to decide which to use
; - Additional arguments beyond the first 4 are placed on the stack, starting at
;   [RSP+32] (above the mandatory 32-byte shadow space), in left-to-right order
; - The caller must allocate at least 32 bytes of shadow space regardless of argc
; - RSP must be 16-byte aligned at the point of the CALL instruction
;
; struct ParamInfo { void *value; void *fp_reg; }
; Layout in memory:
; [elem0.value][elem0.fp_reg][elem1.value][elem1.fp_reg][elem2.value][elem2.fp_reg]...
;      8B           8B            8B           8B            8B           8B
;
; fp_reg != 0 signals that the slot carries a floating-point value and must be
; loaded into XMMn instead of a GPR.

.code

public ffi_call

; Function: ffi_call
; Signature (Windows x64): void *ffi_call(void *func, const ParamInfo *args, U16 argc, bool stack_only)
;   RCX = func       - pointer to the native function to call
;   RDX = args       - pointer to the ParamInfo array
;   R8  = argc       - number of entries in the array
;   R9  = stack_only - unused, reserved for future use
; Returns: RAX or XMM0, forwarded untouched from the called function
;
; Register allocation (all non-volatile, saved/restored in prologue/epilogue):
;   R12 = func pointer
;   R13 = argc
;   R10 = args base pointer       (volatile, set before any call to volatile regs)
;   R11 = current parameter index (volatile, scratch)
;   RBX = stack arg byte offset   (non-volatile, used only in push_stack_param)
ffi_call proc
    ; --- Prologue ---
    push rbp
    mov rbp, rsp
    ; Save non-volatile registers we are going to use as persistent locals.
    ; All three pushes happen before any dynamic allocation, so their positions
    ; relative to RBP are fixed and we can restore them with MOV in the epilogue.
    push rbx
    push r12
    push r13

    ; Move incoming parameters into non-volatile registers so they survive
    ; across the volatile-register writes done during argument dispatch.
    mov r12, rcx                ; r12 = func pointer
    mov r10, rdx                ; r10 = args base pointer
    mov r13, r8                 ; r13 = argc
    xor r11, r11                ; r11 = current parameter index

    ; --- Register parameter dispatch (params 0..3) ---
    ; Walk params[0..min(argc,4)-1]. For each slot, check fp_reg:
    ; if non-zero, load value into XMMn; otherwise load into the GPR for that slot.
next_reg_param:
    cmp r11, r13
    jge end_reg_param           ; all params processed (or argc <= 4)

    ; Byte offset into the array for the current slot: index * sizeof(ParamInfo) = index * 16
    mov rbx, r11
    shl rbx, 4                  ; rbx = r11 * 16  (reused here as scratch; reset later)

    ; Peek at fp_reg (offset +8 within the struct) to decide integer vs float
    mov rax, [r10 + rbx + 8]
    test rax, rax
    jne load_float_param        ; fp_reg != 0 → floating-point slot

load_int_param:
    ; Only 4 GPRs are available for register arguments; params beyond index 3
    ; fall through to the stack-placement phase below.
    cmp r11, 0
    je reg0
    cmp r11, 1
    je reg1
    cmp r11, 2
    je reg2
    cmp r11, 3
    je reg3
    jmp end_reg_param           ; index >= 4: switch to stack phase

reg0:
    mov rcx, [r10 + rbx]
    jmp next_param
reg1:
    mov rdx, [r10 + rbx]
    jmp next_param
reg2:
    mov r8,  [r10 + rbx]
    jmp next_param
reg3:
    mov r9,  [r10 + rbx]
    jmp next_param

load_float_param:
    ; Mirror of the GPR dispatch above, but targets XMM registers.
    ; Under Win64, a floating-point param at position N uses XMMn and the
    ; corresponding GPR slot (RCX/RDX/R8/R9) is left undefined for fixed-arity
    ; native calls, which is the only case we need to support.
    cmp r11, 0
    je reg_f0
    cmp r11, 1
    je reg_f1
    cmp r11, 2
    je reg_f2
    cmp r11, 3
    je reg_f3
    jmp end_reg_param

reg_f0:
    movsd xmm0, qword ptr [r10 + rbx]
    jmp next_param
reg_f1:
    movsd xmm1, qword ptr [r10 + rbx]
    jmp next_param
reg_f2:
    movsd xmm2, qword ptr [r10 + rbx]
    jmp next_param
reg_f3:
    movsd xmm3, qword ptr [r10 + rbx]

next_param:
    inc r11
    jmp next_reg_param

end_reg_param:
    ; --- Dynamic stack allocation ---
    ; R11 = number of params already dispatched to registers (0..4).
    ; Compute how many params still need stack slots.
    mov rax, r13                ; total argc
    sub rax, r11                ; remaining = argc - reg_params

    test rax, rax
    jle alloc_shadow_only       ; no stack params: allocate shadow space only

    ; Each stack param occupies 8 bytes
    shl rax, 3                  ; bytes_for_stack_params = remaining * 8
    jmp add_shadow

alloc_shadow_only:
    xor rax, rax

add_shadow:
    ; The 32-byte shadow space is mandatory regardless of argument count
    add rax, 32

    ; RSP must be 16-byte aligned at the CALL site. At this point RSP is already
    ; offset by 40 bytes from the pre-call aligned value: 8 (return address pushed
    ; by our caller) + 4 * 8 (push rbp/rbx/r12/r13). 40 mod 16 = 8, so RSP is
    ; misaligned by 8. We round rax up to the next multiple of 16 and then add 8
    ; to compensate, ensuring (RSP - rax) mod 16 = 0 at the call site.
    add rax, 15
    and rax, -16
    add rax, 8

    sub rsp, rax                ; carve out shadow space (+ optional stack args)

    ; --- Stack parameter placement ---
    ; Params beyond the first 4 go at [RSP+32], [RSP+40], ... in order.
    ; R11 already holds the index of the first param that didn't fit in a register.
    cmp r11, r13
    jge call_function           ; no stack params

    xor rbx, rbx                ; rbx = byte offset within the stack arg area

push_stack_param:
    cmp r11, r13
    jge call_function

    mov rax, r11
    shl rax, 4                  ; byte offset into ParamInfo array for this slot

    ; Both integer and float stack params are written as raw 8-byte values;
    ; the called function reads them by type from its own stack frame.
    mov rax, [r10 + rax]
    mov [rsp + 32 + rbx], rax  ; place above shadow space, in declaration order

    add rbx, 8
    inc r11
    jmp push_stack_param

    ; --- Call ---
call_function:
    call r12                    ; r12 holds the func pointer, no reload needed
    ; Return value is in RAX (integer/pointer) or XMM0 (float/double).
    ; We must not disturb either register before returning to our own caller.

    ; --- Epilogue ---
    ; RSP currently points into the dynamically allocated frame, not to the
    ; saved-register area. Restore non-volatile registers via their known
    ; RBP-relative addresses before collapsing the frame with MOV RSP, RBP.
    mov rbx, [rbp - 8]
    mov r12, [rbp - 16]
    mov r13, [rbp - 24]
    mov rsp, rbp
    pop rbp
    ret
ffi_call endp

end
