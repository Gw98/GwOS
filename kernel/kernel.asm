[bits 32]

;统一栈内数据：如果压入了错误码，就nop，不用操作；如果没有，就压入0。
%define ERROR_CODE nop  
%define ZERO push 0

extern put_str
extern intr_function_table ;存储具体的中断处理函数的数组

section .data
intr_str db "interrupt occur!", 0xa, 0
global intr_entry_table
intr_entry_table: ;中断处理程序地址 数组

%macro VECTOR 2
;(中断向量号， ERROE_CODE)
section .text
intr%1entry:
    %2  ;ERROR_CODE
    ; push 0x07
    ; push intr_str
    ; call put_str
    ; add esp, 8 ;恢复

    push ds
    push es
    push fs
    push gs
    pushad 

    mov al, 0x20 ;发送中断结束命令EOI, 20h=100000b(EOI位=1，其余位为0)
    out 0xa0, al ;向从片发送
    out 0x20, al ;向主片发送

    push %1 ;参数压栈
    call [intr_function_table + %1 * 4] ;一个函数地址占4B
    jmp intr_exit
    

    ; add esp, 4 
    ; iret

section .data
    dd intr%1entry  ;利用编译器会将同一类型的section归为一个segment，在intr_entry_table数组中依次定义了各个intrXXentry
%endmacro

section .text
global intr_exit
intr_exit:
    add esp, 4 ;跳过参数
    popad
    pop gs
    pop fs
    pop es 
    pop ds 
    add esp, 4 ;跳过ERROR_CODE
    iretd


;中断0x20 ~ 0x30
VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO 
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO 
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE 
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO 
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO 
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO 
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE 
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO 
VECTOR 0x20,ZERO	;时钟中断对应的入口
VECTOR 0x21,ZERO	;键盘中断对应的入口
VECTOR 0x22,ZERO	;级联用的
VECTOR 0x23,ZERO	;串口2对应的入口
VECTOR 0x24,ZERO	;串口1对应的入口
VECTOR 0x25,ZERO	;并口2对应的入口
VECTOR 0x26,ZERO	;软盘对应的入口
VECTOR 0x27,ZERO	;并口1对应的入口
VECTOR 0x28,ZERO	;实时时钟对应的入口
VECTOR 0x29,ZERO	;重定向
VECTOR 0x2a,ZERO	;保留
VECTOR 0x2b,ZERO	;保留
VECTOR 0x2c,ZERO	;ps/2鼠标
VECTOR 0x2d,ZERO	;fpu浮点单元异常
VECTOR 0x2e,ZERO	;硬盘
VECTOR 0x2f,ZERO	;保留

