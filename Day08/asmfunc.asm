; asmfunc.asm
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
    mov rsp, kernel_main_tack + 1024*1024
    call KernelMainNewStack
.fin
    hlt
    jmp .fin


global IoOut32 ; void IoOut32(uint16_t addr, uint32_t data);
IoOut32:
    mov dx, di      ; dx = addr
    mov eax, esi    ; eax = data
    out dx, eax
    ret

global IoIn32 ; void IoIn32(uint16_t addr);
IoIn32:
    mov dx, di      ; dx = addr
    in eax, dx
    ret

global LoadIDT ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di   ; limit
    mov [rsp+2], rs ; offset
    lidt [rsp]
    mov rsp, rbp
    pop rbp
    ret

global LoadGDT ; void LoadGDT(uint16_t limit, uint64_t offset);
LoadGDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di       ; limit
    mov [rsp+2], rsi    ; offset 
    lgdt [rsp]
    mov rsp, rbp
    pop rbp
    ret

global SetDSAll ; void SetDSAll(uint16_t value);
SetDSAll:
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

global SetCSSS ; void SetCSSS(uint16_t cs, uint16_t ss);
SetCSSS:
    push rbp
    mov rbp, rsp
    mov ss, si
    mov rax, .next
    push rdi ; cs
    push rax ; RIP
    o64 retf
.next:
    mov rsp, rbp
    pop rbp
    ret

global SetCR3 ; void SetCR3(uint64_t value);
SetCR3:
    mov cr3, rdi
    ret