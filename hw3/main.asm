    bits 64
    extern malloc, puts, printf, fflush, abort, free
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8 

    section   .text
;;; print_int proc
print_int:    
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf        

    xor rdi, rdi
    call fflush

    mov rsp, rbp
    pop rbp
    ret

;;; p proc     
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
add_element:   
              
             
    push rbp   
    push rbx   
    push r14   
    mov rbp, rsp  
    sub rsp, 16    

    mov r14, rdi 
    mov rbx, rsi

    mov rdi, 16
    call malloc
    test rax, rax      
    jz abort
                      
    mov [rax], r14   
    mov [rax + 8], rbx   

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc
m:                    ; оптимизация: замена рекурсивных вызовов на цикл 
    push rbp
    mov rbp, rsp
    sub rsp, 16

    push rbp
    push rbx
loopm:
    test rdi, rdi    
    jz outm

    mov rbp, rsi     
    mov rbx, rdi    
    mov rdi, [rdi] 
    call rsi      

    mov rdi, [rbx + 8] 
    mov rsi, rbp      
    jmp loopm

    
outm:
    pop rbx
    pop rbp

    mov rsp, rbp
    pop rbp
    ret

;;; f proc
f:                    ; оптимизация: замена рекурсивных вызовов на цикл 
    push rbx         
    push r12
    push r13
loopf:
    mov rax, rsi
    test rdi, rdi  
    jz outf

    mov rbx, rdi      
    mov r12, rsi     
    mov r13, rdx    

    mov rdi, [rdi]    
    call rdx       
    test rax, rax 
    jz z

    mov rdi, [rbx] 
    mov rsi, r12
    call add_element
    mov rsi, rax
    jmp ff

z:
    mov rsi, r12

ff:
    mov rdi, [rbx + 8]
    mov rdx, r13
    jmp loopf
    ;call f

outf:
    pop r13
    pop r12
    pop rbx

    ret

;;; main proc
main:
    push rbx

    xor rax, rax
    mov rbx, data_length
adding_loop:                      
    mov rdi, [data - 8 + rbx * 8]
    mov rsi, rax                   
    call add_element               
    dec rbx                        
    jnz adding_loop                 
    mov rbx, rax              

    mov rdi, rax                
    mov rsi, print_int         
    call m                    

    mov rdi, empty_str       
    call puts               

    mov rdx, p             
    xor rsi, rsi          
    mov rdi, rbx         
    call f              

    mov rdi, rax       
    mov rsi, print_int 
    call m            
                         ; массив с нечетными числами сейчас - в rax
    call free            ; освобождаем память из-под массива для нечетных чисел
    mov rax, rbx         ; копия массива data сейчас - в rbx
    call free            ; записываем его адрес в rax и тоже освобождаем

    mov rdi, empty_str            
    call puts                

    pop rbx                         

    xor rax, rax
    ret
