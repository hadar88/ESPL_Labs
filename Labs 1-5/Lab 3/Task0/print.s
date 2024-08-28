section .data
    hello db 'hello world', 0xA ; make the word and add a new line character

section .text
    global _start

_start:
    mov eax, 4 ; sys_write
    mov ebx, 1 ; stdout
    mov ecx, hello ; address of the string
    mov edx, 12 ; length of the string
    int 0x80 ; call kernel
    
    mov eax, 1 ; sys_exit
    int 0x80 ; call kernel