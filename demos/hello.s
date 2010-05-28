
.text

.global _start

_start:
    la $a0, hello_str
    
    addiu $v0, $zero, 4
    syscall
    
    break
    
.data
hello_str: .asciz "Hello world!\n"
