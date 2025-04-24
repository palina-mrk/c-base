    bits 64
    extern malloc, puts, printf, fflush, abort
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42  ; массив 8-байтных чисел (long int)
data_length: equ ($-data) / 8  ; длина массива

    section   .text
;;; print_int proc
print_int:                  ; вывод на экран числа
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf         ; printf(char* "%ld \0", long int n)

    xor rdi, rdi
    call fflush

    mov rsp, rbp
    pop rbp
    ret

;;; p proc      ; возвращает 1, если число нечетно, 0 если четно
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
add_element:    ; add_element(long int x, long int* arr)
                ; добавляет спереди к массиву arr еще 8 байт
                ; и записывает туда х
    push rbp   
    push rbx   
    push r14   
    mov rbp, rsp  
    sub rsp, 16    

    mov r14, rdi   ; а здесь - значение, которое добавляем
    mov rbx, rsi   ; здесь будет адрес уже созданного массива 

    mov rdi, 16
    call malloc           ;  (*rax) = malloc(16 бит)
    test rax, rax         ;  if ((*rax) != NULL)
    jz abort
                          ; rax = адрес с выделенными 2 байтами
    mov [rax], r14        ; (*rax)[0] = *rdi
    mov [rax + 8], rbx    ; (*rax)[1] = *rsi

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc
m:                     ; здесь: печать массива 8-байтных целых
                       ; а вообще: m (long int* arr, void proc(long int))
                       ; реализует применение процедуры 
                       ; proc к каждому эл-ту массива arr,
                       ; но делает это рекурсивно: пока arr != NULL,
                       ; выполняет proc(*arr), затем вызывает m(arr+1, proc)
    push rbp
    mov rbp, rsp
    sub rsp, 16

    test rdi, rdi       ; если аргумент %1 == 0, return
    jz outm

    push rbp
    push rbx

    mov rbx, rdi       ; rbx = %1
    mov rbp, rsi       ; rbp = %2

    mov rdi, [rdi]    ; запоминаем некий адрес в rdi
    call rsi          ; call %2

    mov rdi, [rbx + 8]  ; rdi++
    mov rsi, rbp
    call m              ; рекурсия

    pop rbx
    pop rbp

outm:
    mov rsp, rbp
    pop rbp
    ret

;;; f proc
f:                   ; long int* f(long int* arr, long int, int p(long int))
                     ; формирует "задом наперёд" новый массив new_arr:
                     ; идёт вперед по массиву arr, если p(arr[i]) == 1,
                     ; то добавляет СПЕРЕДИ к new_arr элемент arr[i].
                     ; В нашем случает - формирует массив из нечетных элементов
                     ; исходного массива, записанных "задом наперед".
                     ; как и функция m, действует рекурсивно

    mov rax, rsi

    test rdi, rdi    ; %1 == 0 -> return
    jz outf

    push rbx         
    push r12
    push r13

    mov rbx, rdi      ; %1 -> arr
    mov r12, rsi      ; %2 -> 0
    mov r13, rdx      ; %3 -> p()

    mov rdi, [rdi]    
    call rdx          ; p(*rdi)
    test rax, rax     ; если *rdi четно -> переход к следующему эл-ту
    jz z

    mov rdi, [rbx]    ; (формируем новый массив) и добавляем в него *rdi 
    mov rsi, r12
    call add_element
    mov rsi, rax
    jmp ff

z:
    mov rsi, r12

ff:
    mov rdi, [rbx + 8]
    mov rdx, r13
    call f

    pop r13
    pop r12
    pop rbx

outf:
    ret

;;; main proc
main:
    push rbx

    xor rax, rax
    mov rbx, data_length
adding_loop:                        ; создаём копию массива data в rax 
    mov rdi, [data - 8 + rbx * 8]   ; 
    mov rsi, rax                   
    call add_element               
    dec rbx                        
    jnz adding_loop                 
    mov rbx, rax              

    mov rdi, rax                    ; *rdi = адрес массива
    mov rsi, print_int              ; *rsi = 'print_int'
    call m                          ; m(массив 16-байтных, print_int)

    mov rdi, empty_str              ; *rdi = 0x0
    call puts                       ; вывод '\0'

    mov rdx, p                      ; *rdx = 'p'
    xor rsi, rsi                    ; *rsi = 0
    mov rdi, rbx                    ; *rdi = адрес массива
    call f                          ; f(массив, 0, 'p')

    mov rdi, rax                    ; *rdi = адрес массива
    mov rsi, print_int              ; *rsi = 'print_int'
    call m                          ; m(массив, print_int)

    mov rdi, empty_str              ; *rdi = 0x0
    call puts                       ; вывод '\0'

    pop rbx                         

    xor rax, rax
    ret
