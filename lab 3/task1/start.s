WRITE EQU 4
READ EQU 3

STDOUT EQU 1
STDERR EQU 2

section .data
    newline db 10   ; ascii code for newline
    infile dd 0     ;stdin
    outfile dd 1    ; stdout
    buffer db 0
    flag_i db "-i", 0     ; Input file flag
    flag_o db "-o", 0     ; Output file flag


section .text
global _start
global system_call
global main
extern strlen
extern strcmp

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

section .text
main:
    ;cdecl C calling - prologue
    push ebp
    mov ebp, esp

    ;get argc, argv
    mov ecx, [ebp+8]    ; argc
    mov edx, [ebp+12]   ; argv
   

PRINT:
    push ecx    ; save ecx
    push edx    ; save edx
    
    mov esi, [edx]      ; argv[i]

    ;get strlen
    push dword esi    ; esi holds the value of argv[i] and strlen expects char*
    call strlen
    add esp, 4

    ;print arg[i]
    mov ecx, esi    ; the address of the message
    mov edx, eax    ; length of the message: strlen
    mov ebx, STDERR ; file discriptor
    mov eax, WRITE  ; sys_call number
    int 0x80       ; make the system call

    ;print newline
    mov ecx, newline
    mov edx, 1
    mov ebx, STDERR
    mov eax, WRITE
    int 0x80


check_input:
    cmp byte [esi], '-'                     
    jne check_output                           
    cmp byte [esi + 1], 'i'                 
    jne check_output

    ; open
    add esi, 2  ;skip '-i'
    mov eax, 5  ; open sys_call
    mov ebx, esi
    mov ecx, 0  ; read only
    mov edx, 0644o
    int 0x80

    ;error handling
    cmp eax, 0
    jl exit
    
    ;get fd
    mov [infile], eax

    jmp NEXT    ;if the argument is input then no need to check if it is output

check_output:
    cmp byte [esi], '-'                     
    jne NEXT                           
    cmp byte [esi + 1], 'o'                 
    jne NEXT

    ; open
    add esi, 2  ;skip '-o'
    mov eax, 5  ; open sys_call
    mov ebx, esi
    mov ecx, 2  ; write only
    mov edx, 0644o
    int 0x80

    ;error handling
    cmp eax, 0
    jl close_files_and_exit
    
    ;get fd
    mov [outfile], eax


NEXT:
    pop edx     ; get ecx prev value
    pop ecx     ; get edx prev value
    
    ;move to the next argument if exists
    add edx, 4   ; advance edx to &arg[i+1]
    dec ecx     ; argc--
    jnz PRINT    ; check flags from dec(if argc != 0)

Read_and_encode:
    ; Read a char
    mov eax, READ
    mov ebx, [infile]
    mov ecx, buffer
    mov edx, 1
    int 0x80

    ; If EOF then exit 
    cmp eax, 0  ; eax - number of bytes successfully read
    je close_files_and_exit

    ; Else encode and print
    mov al, [buffer]   ; save the char

check_char:
    cmp al, 'A'     ; if less then A don't encode
    jl write
    cmp al, 'Z'     ; if in [A,Z]
    jle encode

    cmp al, 'a' 
    jl write
    cmp al, 'z'
    jg write

encode: 
    inc al
    mov [buffer], al   ; save the encoded char
    
write:

    mov eax, WRITE
    mov ebx, [outfile]
    mov ecx, buffer
    mov edx, 1
    int 0x80

    ; continue reading until EOF
    jmp Read_and_encode

close_files_and_exit:
close_input:
    cmp byte [infile], 0
    je close_outfile
    mov eax, 6
    mov ebx, [infile]
    int 0x80

close_outfile:
    cmp byte [outfile], 1
    je exit
    mov eax, 6
    mov ebx, [outfile]
    int 0x80

;cdecl C calling - epilogue & file closing
exit:
    mov esp, ebp                            ; Restore stack frame
    pop ebp
    xor eax, eax                            ; Return 0
    ret
    

    