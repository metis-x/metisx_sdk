
.equ DTCM_BASE,       ( 0x30000000 )
.equ DTCM_SIZE,       ( 16 * 1024 )
.equ STACK_BASE_ADDR, ( DTCM_BASE | DTCM_SIZE )

.global _start
_start:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    mv      x0, x0
    mv      x1, x0
    mv      x2, x0
    mv      x3, x0
    mv      x4, x0
    mv      x5, x0
    mv      x6, x0
    mv      x7, x0
    mv      x8, x0
    mv      x9, x0
    mv      x10, x0
    mv      x11, x0
    mv      x12, x0
    mv      x13, x0
    mv      x14, x0
    mv      x15, x0
    li      sp, STACK_BASE_ADDR
    call    _main
    ret