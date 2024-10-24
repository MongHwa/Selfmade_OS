extern kernel_main_stack
extern kernelMainNewStack

global KernelMain
KernelMain:
    mov rsp, kernel_main_stack + 1024*1024
    call kernelMainNewStack
.fin:
    hlt
    jmp .fin

global LoadGDT ; from segment.cpp / void LoadGDT(uint16_t limit, uint64_t offset);
LoadGDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di ; limit
    mov [rsp+2], rsi ; offset
    lgdt [rsp]
    mov rsp, rbp
    pop rbp
    ret