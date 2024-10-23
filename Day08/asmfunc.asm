extern kernel_main_stack
extern kernelMainNewStack

global KernelMain
KernelMain:
    mov rsp, kernel_main_stack + 1024*1024
    call kernelMainNewStack
.fin:
    hlt
    jmp .fin