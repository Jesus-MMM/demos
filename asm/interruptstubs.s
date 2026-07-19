.set IRO_BASE, 0x20

.section .text

.extern handle_interrupt

.macro handle_interrupt_exception_no_err num
.global handle_interrupt_exception\num
handle_interrupt_exception\num:
    pushl $0
    movl $\num, (interruptnumber)
    jmp int_bottom
.endm

.macro handle_interrupt_exception_err num
.global handle_interrupt_exception\num
handle_interrupt_exception\num:
    movl $\num, (interruptnumber)
    jmp int_bottom
.endm

.macro handle_interrupt_request num
.global handle_interrupt_request\num
handle_interrupt_request\num:
    pushl $0
    movl $\num + IRO_BASE, (interruptnumber)
    jmp int_bottom
.endm

handle_interrupt_request 0x00
handle_interrupt_request 0x01
handle_interrupt_request 0x0C

handle_interrupt_exception_no_err 0x00
handle_interrupt_exception_no_err 0x01
handle_interrupt_exception_no_err 0x02
handle_interrupt_exception_no_err 0x03
handle_interrupt_exception_no_err 0x04
handle_interrupt_exception_no_err 0x05
handle_interrupt_exception_no_err 0x06
handle_interrupt_exception_no_err 0x07
handle_interrupt_exception_err 0x08
handle_interrupt_exception_no_err 0x09
handle_interrupt_exception_err 0x0A
handle_interrupt_exception_err 0x0B
handle_interrupt_exception_err 0x0C
handle_interrupt_exception_err 0x0D
handle_interrupt_exception_err 0x0E
handle_interrupt_exception_no_err 0x0F
handle_interrupt_exception_no_err 0x10
handle_interrupt_exception_err 0x11
handle_interrupt_exception_no_err 0x12
handle_interrupt_exception_no_err 0x13
handle_interrupt_exception_no_err 0x14
handle_interrupt_exception_no_err 0x15
handle_interrupt_exception_no_err 0x16
handle_interrupt_exception_no_err 0x17
handle_interrupt_exception_no_err 0x18
handle_interrupt_exception_no_err 0x19
handle_interrupt_exception_no_err 0x1A
handle_interrupt_exception_no_err 0x1B
handle_interrupt_exception_no_err 0x1C
handle_interrupt_exception_no_err 0x1D
handle_interrupt_exception_no_err 0x1E
handle_interrupt_exception_no_err 0x1F

.macro exception_table_entry num
.long handle_interrupt_exception\num
.endm

.global exception_handler_table
exception_handler_table:
    exception_table_entry 0x00
    exception_table_entry 0x01
    exception_table_entry 0x02
    exception_table_entry 0x03
    exception_table_entry 0x04
    exception_table_entry 0x05
    exception_table_entry 0x06
    exception_table_entry 0x07
    exception_table_entry 0x08
    exception_table_entry 0x09
    exception_table_entry 0x0A
    exception_table_entry 0x0B
    exception_table_entry 0x0C
    exception_table_entry 0x0D
    exception_table_entry 0x0E
    exception_table_entry 0x0F
    exception_table_entry 0x10
    exception_table_entry 0x11
    exception_table_entry 0x12
    exception_table_entry 0x13
    exception_table_entry 0x14
    exception_table_entry 0x15
    exception_table_entry 0x16
    exception_table_entry 0x17
    exception_table_entry 0x18
    exception_table_entry 0x19
    exception_table_entry 0x1A
    exception_table_entry 0x1B
    exception_table_entry 0x1C
    exception_table_entry 0x1D
    exception_table_entry 0x1E
    exception_table_entry 0x1F

int_bottom:
    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs

    push %esp
    movl (interruptnumber), %eax
    pushl %eax
    call handle_interrupt

    movl %eax, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $4, %esp
    iret

.globl ignore_interrupt_request
ignore_interrupt_request:
    pushl %eax
    movb $0x20, %al
    outb %al, $0x20
    outb %al, $0xA0
    popl %eax
    iret

.data
    interruptnumber: .long 0
