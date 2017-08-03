
.CODE

GetProcId PROC
    push rdi
    push rbx
    mov rdi, rcx
    mov rax, 080000000h
    cpuid
    cmp rax, 080000004h
    jl zakoncz
    mov rax, 080000002h
    cpuid
    stosd
    mov rax, rbx
    stosd
    mov rax, rcx
    stosd
    mov rax, rdx
    stosd
    mov rax, 080000003h
    cpuid
    stosd
    mov rax, rbx
    stosd
    mov rax, rcx
    stosd
    mov rax, rdx
    stosd
    mov rax, 080000004h
    cpuid
    stosd
    mov rax, rbx
    stosd
    mov rax, rcx
    stosd
    mov rax, rdx
    stosd
zakoncz:
    pop rbx
    pop rdi
    ret
GetProcId ENDP
END
