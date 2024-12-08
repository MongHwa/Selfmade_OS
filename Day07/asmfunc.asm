; asmfunc.asm
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text
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
    mov[rsp], di    ; limit
    mov[rsp+2], rs  ; offset
    lidt [rsp]
    mov rsp, rbp
    pop rbp
    ret