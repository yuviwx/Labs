section .data
    format db "%d", 10, 0   ; null terminated string - 10 is new line's ascii code

section .text
extern printf, puts
global main

main:
    ;C CDECL calling convention - prologue 
    push    ebp                         ; 1. save acf
    mov     ebp, esp                    ; 2. create new acf
    pushad                              ; 3. save reg

    ; save argc, argv
    mov     eax, [ebp + 8]              ;argc
    mov     ebx, [ebp + 12]             ;argv

    ; save eax, ebx before printf
    push ebx
    push eax

    ;printf("%d", argc)
    push    eax                         ; argc
    push    format                      ; "%d"
    call    printf
    add     esp, 8                      ; clean up the stack

    ; get back argc and argv
    pop eax
    pop ebx

    xor     ecx, ecx                    ; create a counter(ecx=0)
    jmp     check_argc

print_arg: 
    ; save eax, ebx, ecx before puts
    pushad

    ;puts(const char * str)
    push    dword [ebx + ecx * 4]       ; argv[i]
    call    puts
    add     esp, 4

    ; get back argc, argv, counter
    popad

    ; increase counter
    inc     ecx                         ; counter++ 


check_argc:
    cmp eax, ecx                        ; Compare argc (eax) with counter (ecx)
    jne print_arg                        ; keep printing while counter < argc

    ;C CDECL calling convention - epilogue
    popad
    mov esp, ebp                        ; return the top of the stack to the previous activation frame top
    pop ebp                             ; return ebp to the base of the previous frame base
    xor eax, eax                        ; C convention, 0 return value means success 
    ret                                 ; pop the return address that was pushed by the call instruction and jump there




