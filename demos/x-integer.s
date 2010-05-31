
.text
.global _start

_start:
    LI $8, 0xA1234567
    LI $9, 0xA1234567
    ADD $10, $8, $9
    ADDU $11, $8, $9
    sub $10, $zero, $8
    subu $10, $zero, $8
    sub $11, $9, $zero
    
    break
