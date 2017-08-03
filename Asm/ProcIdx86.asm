
.MODEL FLAT,C
.CODE

GetProcId PROC
    push edi
    push ebx
    mov edi, [esp+0Ch]
    mov eax, 080000000h
    cpuid
    cmp eax, 080000004h
    jl zakoncz
    mov eax, 080000002h
    cpuid
    stosd
    mov eax, ebx
    stosd
    mov eax, ecx
    stosd
    mov eax, edx
    stosd
    mov eax, 080000003h
    cpuid
    stosd
    mov eax, ebx
    stosd
    mov eax, ecx
    stosd
    mov eax, edx
    stosd
    mov eax, 080000004h
    cpuid
    stosd
    mov eax, ebx
    stosd
    mov eax, ecx
    stosd
    mov eax, edx
    stosd
zakoncz:
    pop ebx
    pop edi
    ret
GetProcId ENDP
END
