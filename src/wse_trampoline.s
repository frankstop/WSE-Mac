.text
.globl _wse_opcode_trampoline
.globl _g_statement_done_target
.globl _g_unhandled_target
.globl _g_error_string_ptr

.p2align 4
_wse_opcode_trampoline:
    # 1. Save all CPU registers
    pushfq
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # 2. Pass %rbp (frame pointer of operation_Execute) to C++ dispatcher in %rdi
    movq %rbp, %rdi
    callq _wse_handle_opcode_dispatch

    # Return value in %eax:
    # 1 = handled by custom WSE opcode handler -> jump to statement done
    # 0 = unhandled -> jump to original error handler
    testl %eax, %eax
    jnz .L_wse_handled

    # --- UNHANDLED PATH ---
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    popfq

    # Advance to statement completion to prevent crashing on unhandled opcodes
    # and allow game to continue safely
    movq _g_statement_done_target(%rip), %rax
    jmpq *%rax

.L_wse_handled:
    # --- WSE HANDLED PATH ---
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    popfq

    # Jump straight to statement done (0x100adcd5d + slide)
    movq _g_statement_done_target(%rip), %rax
    jmpq *%rax

.data
.p2align 3
_g_statement_done_target:
    .quad 0
_g_unhandled_target:
    .quad 0
_g_error_string_ptr:
    .quad 0
