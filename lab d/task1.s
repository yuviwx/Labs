; multi_precision number is a number that is larger than the conventional numbers in the processor and so it's stracture in the memroy is an array of words in little endian order.
; struct multi{
;   unsigned char size;
;   unsigned char num [];
; }


section .data
    format          db "%04hx", 0
    new_line        db 10, 0
    prompt          db "enter hex: ", 0
    flag            db 0
    padding         db 0
    x_struct: dw 5                              ; everything is in units of words.
    x_num: dw 0x00aa, 1,2,0x0044,0x004f         ; since char array stays in the same order, the words are little endian from the start, 
                                                ; meaning the acutal number is 004f00440002000100aa

section .bss
    buffer    resb 600          ; Buffer to store input from fgets

; ============================ SECTION .TXT ==========================================

section .text
    extern printf, fgets, stdin, malloc
    global main

; ============================ MAIN ==========================================

main:
    ; CDECL prologue
    push    ebp
    mov     ebp, esp

    ;call getmulti

    ; Call print_multi with pointer to struct
    ;push    eax                         ; Push argument (pointer to struct)
    push    x_struct
    call    print_multi                 ; Call the function
    add     esp, 4                      ; Clean up stack

    ; CDECL epilogue
    mov     esp, ebp
    pop     ebp
    xor     eax, eax                        ; C convention, 0 return value means success 
    ret                                 ; Return to caller

; ============================ 1.A - PRINT_MULTI ==========================================

print_multi:
    ; CDECL prologue 
    push    ebp                         ; 1. save acf
    mov     ebp, esp                    ; 2. create new acf
    pushad                              ; 3. save reg

    ; get p, and dereference size and num
    mov     eax, [ebp + 8]              ; struct multi* p
    movzx   ebx, byte [eax]             ; Zero-extend size into ebx (loop counter)
    add     eax, ebx                    
    add     eax, ebx                    ; &num[-1] - ebx represent the number of words not bytes, so we need to jump twice.

    jmp     check_size

print_word:
    ; save eax, ebx before printf
    push ebx
    push eax

    ;printf("%04hx", argc)
    movzx   ecx, word [eax]             ; Load 2 bytes (word) and zero-extend into ecx
    push    ecx                         ; word
    push    format                      ; "%04hx"
    call    printf
    add     esp, 8                      ; clean up the stack

    ; get back argc and argv
    pop eax
    pop ebx

    ; decrease size
    dec     ebx                         ; size--
    sub     eax, 2                      ; advance pointer by a word(16 bytes)


check_size:
    cmp ebx, 0                         ; check the number of words left to print
    jg print_word                      ; keep printing while counter < argc

print_new_line:
    push    new_line                    ; "\n"
    call    printf
    add     esp, 4                      ; clean up the stack

    jmp exit

; ============================ 1.B - GET_MULTI ==========================================

getmulti:                               ; a function that reads line and store it in struct print_multi
    ; CDECL prologue 
    push    ebp                         ; 1. save acf
    mov     ebp, esp                    ; 2. create new acf
    pushad                              ; 3. save reg

; ========== get_input ==========
    ; prompt the user
    push prompt
    call printf
    add esp, 4

    mov eax, buffer
    ; fgets(char *str, int n, FILE *stream)
    push    dword [stdin]               ; FILE *stream
    push    600                         ; int n - maximum number of characters to read
    push    eax                         ; char *str - char array pointer to store the input
    call    fgets
    add     esp, 12                     ; clean up the stack

    ; check for errors
    cmp     eax, 0                      ; fgets return null on fail
    je      exit

; ========== calc_length ========== ;
    xor     ebx, ebx                    ; counter

calc_length:                            ; Count bytes
    cmp     byte [eax + ebx], 10        ; Check for new_line - the end of fgets
    je      allocate_memory
    inc     ebx
    jmp     calc_length

; ========== mod4 ========== ;
test_even:
    test    ebx, 1
    jz      test_4
    inc     ebx
    inc     padding

test_4:
    test    ebx, 4
    jnz     allocate_memory
    add     ebx, 2
    add     padding, 2

; ========== allocate_memory ========== ;
allocate_memory:
    shr         ebx, 1                  ; ebx is char count and every 2 chars are one byte
    add         ebx,2                   ; struct multi also has one unit(word) for size
    push        ebx                     ; malloc input_length bytes(ebx)
    call        malloc
    add         esp, 4

    sub         ebx, 2                  ; without 'size'
    shr         ebx, 1                  ; every unit is 2 bytes
    mov         [eax], bx               ; save the size - number of words in the multi_precision number
    add         eax, 2                  ; move pointer by one unit(word)

    shl         ebx, 2                  

; each char is a hex number, each hex number is 4 bits, 
; so we'll take 2 chars(2hex=8bits) each time and turn them to 1 byte represanting a hexnumber.
; but we will need to save more than that, each unit is a word.
; ========== store_hex ========== ;
    mov     esi, buffer + ebx*4 - 4     ; buffer[-2] point to the last word.

check_odd:                              ; check if buffer size is odd
    cmp     odd, 1
    jz      store_loop                  ; 0 => even

    ; if odd - padd the first byte with 0 and push
    mov     ch, 0
    mov     cl, byte [esi]
    inc     ebx                         ; update ebx - counter, so we'll know how much back to go with the pointer
    dec     esi                         ; we only used 1 byte from input - later we add 2
    dec     edx                         ; edx - counter that will go up until ebx, we add 2 later
    jmp     string_to_hex


store_loop:                             
    cmp     ebx, edx
    je      ret_exit
    mov     cx, [esi]                   ; [esi] - buffer[i], take 2 bytes from buffer
    jmp     string_to_hex               
continue_storing:
    mov     [eax], cl                   ; push 2 bytes to struct
    inc     eax
    add     esi, 2
    inc     edx
    jmp     store_loop

; ========== string_to_hex ========== ;
string_to_hex:
    push    ebx                         ; save ebx
    xor     ebx, ebx                    ; reset ebx
; cx contains 2 chars(16bits) that we need to convert to 1 hexnumber(8bits)
; input: '1' '0' -> cx: '0' '1' -> bits(ascii values): 0001 1110 0001 1111 -> hex_representation: 0001 0000
; notice 8 bits is only half of cx, so we'll save the value in cl.

;1. convert cl - V
;2. mov bl, cl - V
;3. shl bl, 4 - V
;4. mov cl, ch - V

;5. convert cl - V
;6. add bl, cl - V
;7. mov bl, cl - V

char_crossroads:
    cmp     cl, '9'
    jle     convert_number              ; '0'-'9' case
    cmp     cl, 'F'
    jle     convert_uppercase           ; 'A'-'F' case
    cmp     cl, 'f'
    jle     convert_lowercase           ; 'a'-'f' case

convert_number:                         ; bl = bl - '0'
    sub     cl, '0'
    jmp     finish_converting

convert_uppercase:                      ; bl = bl - 'A' + 10
    sub     cl, 'A'
    add     cl, 10
    jmp     finish_converting

convert_lowercase:                      ; bl = bl - 'a' + 10
    sub     cl, 'a'
    add     cl, 10

finish_converting:
    cmp     byte [flag], 1
    je      ret_storing
    inc     byte [flag]
    mov     bl, cl
    shl     bl, 4
    mov     cl, ch
    jmp     char_crossroads

ret_storing:
    add     bl, cl
    mov     cl, bl
    pop     ebx
    dec     byte [flag]
    jmp     continue_storing


ret_exit:
    ; return the multi struct we built by putting it into eax.
    inc     ebx                         ; struct is size and num, ebx only is the length of num, add 1 byte for size
    sub     eax, ebx                    ; reset the pointer to the base of the struct
    mov [esp+28], eax  ; Save return value in correct `popad` location

exit:
    ;C CDECL calling convention - epilogue
    popad
    mov esp, ebp                        ; return the top of the stack to the previous activation frame top
    pop ebp                             ; return ebp to the base of the previous frame base
    ret                                 ; pop the return address that was pushed by the call instruction and jump there



   