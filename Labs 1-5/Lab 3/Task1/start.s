section .data
    newline db 0xA ; new line character
    Infile dd 0 ; File descriptor for the input file
    Outfile dd 1 ; File descriptor for the output file
    
section .bss
    buffer resb 1 ; Reserve a 1 byte buffer for reading input

section .text
    global _start
    extern strlen

_start:

main:
    mov ecx, [esp + 4] ; Load the current argument 
    add esp, 4         ; Move to the next argument

    test ecx, ecx      ; Check if the pointer is null
    jz encode      ; If yes, start encoding
    
    push ecx           ; Save the pointer to the string
    call strlen        ; Get the length of the string
    
    mov edx, eax       ; Move the length to edx

    mov eax, 4         ; sys_write
    mov ebx, dword [Outfile] ; stdout
    pop ecx            ; Restore the pointer to the string
    int 0x80           ; Call kernel

    push ecx

    mov eax, 4         ; sys_write
    mov ebx, dword [Outfile] ; stdout
    mov ecx, newline   ; Load the new line character
    mov edx, 1         ; Length of the new line character
    int 0x80           ; Call kernel

    pop ecx           ; Restore the pointer to the string

    push ecx          ; Save the pointer to the string
    call handle_files ; Handle the input and output files
    pop ecx           ; Restore the pointer to the string

    jmp main           ; Repeat

encode: 
    mov eax, 3         ; sys_read
    mov ebx, dword [Infile] ; File descriptor for the input file
    mov ecx, buffer    ; Buffer to read the input
    mov edx, 1         ; Read 1 byte
    int 0x80           ; Call kernel

    cmp eax, 0         ; Check if the end of file is reached
    je exit_program    ; If yes, exit

    mov al, [buffer]   ; Load the byte to al

    cmp al, 'A'      
    jl print          ; If the byte is less than 'A', print it
    
    cmp al, 'z'
    jg print         ; If the byte is greater than 'Z', print it

    inc al           ; Increment the byte by 1

    jmp print      ; Print the byte

print:
    mov [buffer], al
    mov eax, 4         ; sys_write
    mov ebx, dword [Outfile] ; stdout
    mov ecx, buffer    ; Buffer to write the output
    mov edx, 1         ; Length of the buffer
    int 0x80           ; Call kernel

    mov eax, 4         ; sys_write
    mov ebx, dword [Outfile] ; stdout
    mov ecx, newline   ; Load the new line character
    mov edx, 1         ; Length of the new line character
    int 0x80           ; Call kernel

    jmp encode        ; Repeat

handle_files:
    
    cmp byte [ecx], '-' ; Check if the first character is '-'
    jne exit_handle_files ; If not, exit

    cmp byte [ecx + 1], 'i' ; Check if the second character is 'i'
    je handle_input_file ; If yes, handle input file

    cmp byte [ecx + 1], 'o' ; Check if the second character is 'o'
    je handle_output_file ; If yes, handle output file

    ret 

handle_input_file:
    add ecx, 2 ; Skip the '-' and 'i' characters

    mov eax, 5 ; sys_open
    mov ebx, ecx ; Filename
    mov ecx, 2 ; O_RDONLY
    mov edx, 0 
    int 0x80 ; Call kernel

    cmp eax, -1 ; Check if the file was opened successfully
    je exit_handle_files ; If not, exit
    
    mov [Infile], eax ; Save the file descriptor
    
    ret

handle_output_file:
    add ecx, 2 ; Skip the '-' and 'o' characters

    mov eax, 5 ; sys_open
    mov ebx, ecx ; Filename
    mov ecx, 577 ; O_WRONLY
    mov edx, 511
    int 0x80 ; Call kernel

    cmp eax, -1  ; Check if the file was opened successfully
    je exit_handle_files ; If not, exit
    mov [Outfile], eax ; Save the file descriptor

    ret

exit_handle_files:
    ret

exit_program:
    mov eax, 1         ; sys_exit
    xor ebx, ebx       ; Return 0
    int 0x80           ; Call kernel




