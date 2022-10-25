.global main
itoa:
        addi    sp,sp,-48
        sw      s0,44(sp)
        addi    s0,sp,48
        sw      a0,-36(s0)
        sw      a1,-40(s0)
        lw      a5,-40(s0)
        bge     a5,zero,.L2
        lw      a5,-36(s0)
        addi    a4,a5,1
        sw      a4,-36(s0)
        li      a4,45
        sb      a4,0(a5)
        lw      a5,-40(s0)
        neg     a5,a5
        sw      a5,-40(s0)
.L2:
        lw      a5,-40(s0)
        bne     a5,zero,.L3
        lw      a5,-36(s0)
        addi    a4,a5,1
        sw      a4,-36(s0)
        li      a4,48
        sb      a4,0(a5)
        lw      a5,-36(s0)
        addi    a4,a5,1
        sw      a4,-36(s0)
        sb      zero,0(a5)
        j       .L9
.L3:
        lw      a5,-36(s0)
        sw      a5,-20(s0)
        j       .L5
.L6:
        lw      a4,-40(s0)
        li      a5,10
        rem     a5,a4,a5
        andi    a4,a5,0xff
        lw      a5,-20(s0)
        addi    a3,a5,1
        sw      a3,-20(s0)
        addi    a4,a4,48
        andi    a4,a4,0xff
        sb      a4,0(a5)
        lw      a4,-40(s0)
        li      a5,10
        div     a5,a4,a5
        sw      a5,-40(s0)
.L5:
        lw      a5,-40(s0)
        bne     a5,zero,.L6
        lw      a5,-20(s0)
        addi    a4,a5,-1
        sw      a4,-20(s0)
        sb      zero,0(a5)
        j       .L7
.L8:
        lw      a5,-20(s0)
        lbu     a4,0(a5)
        lw      a5,-36(s0)
        lbu     a5,0(a5)
        xor     a5,a4,a5
        andi    a4,a5,0xff
        lw      a5,-20(s0)
        sb      a4,0(a5)
        lw      a5,-36(s0)
        lbu     a4,0(a5)
        lw      a5,-20(s0)
        lbu     a5,0(a5)
        xor     a5,a4,a5
        andi    a4,a5,0xff
        lw      a5,-36(s0)
        sb      a4,0(a5)
        lw      a5,-20(s0)
        lbu     a4,0(a5)
        lw      a5,-36(s0)
        lbu     a5,0(a5)
        xor     a5,a4,a5
        andi    a4,a5,0xff
        lw      a5,-20(s0)
        sb      a4,0(a5)
        lw      a5,-36(s0)
        addi    a5,a5,1
        sw      a5,-36(s0)
        lw      a5,-20(s0)
        addi    a5,a5,-1
        sw      a5,-20(s0)
.L7:
        lw      a4,-20(s0)
        lw      a5,-36(s0)
        bgtu    a4,a5,.L8
.L9:
        nop
        lw      s0,44(sp)
        addi    sp,sp,48
        jr      ra
write:
        addi    sp,sp,-32
        sw      s0,28(sp)
        addi    s0,sp,32
        sw      a0,-20(s0)
        lw      a5,-20(s0)
        mv      a0, a5
        li      a7, 4
        ecall

        nop
        lw      s0,28(sp)
        addi    sp,sp,32
        jr      ra
vsprintf:
        addi    sp,sp,-304
        sw      ra,300(sp)
        sw      s0,296(sp)
        addi    s0,sp,304
        sw      a0,-292(s0)
        sw      a1,-296(s0)
        sw      a2,-300(s0)
        lw      a5,-292(s0)
        sw      a5,-20(s0)
        j       .L12
.L20:
        lw      a5,-296(s0)
        lbu     a4,0(a5)
        li      a5,37
        beq     a4,a5,.L13
        lw      a5,-20(s0)
        addi    a4,a5,1
        sw      a4,-20(s0)
        lw      a4,-296(s0)
        lbu     a4,0(a4)
        sb      a4,0(a5)
        j       .L14
.L13:
        lw      a5,-296(s0)
        addi    a5,a5,1
        sw      a5,-296(s0)
        lw      a5,-296(s0)
        lbu     a5,0(a5)
        li      a4,100
        beq     a5,a4,.L15
        li      a4,115
        beq     a5,a4,.L21
        j       .L14
.L15:
        lw      a5,-300(s0)
        lw      a5,0(a5)
        mv      a1,a5
        lw      a0,-20(s0)
        call    itoa
        lw      a5,-300(s0)
        addi    a5,a5,4
        sw      a5,-300(s0)
        j       .L18
.L19:
        lw      a5,-20(s0)
        addi    a5,a5,1
        sw      a5,-20(s0)
.L18:
        lw      a5,-20(s0)
        lbu     a5,0(a5)
        bne     a5,zero,.L19
        j       .L14
.L21:
        nop
.L14:
        lw      a5,-296(s0)
        addi    a5,a5,1
        sw      a5,-296(s0)
.L12:
        lw      a5,-296(s0)
        lbu     a5,0(a5)
        bne     a5,zero,.L20
        lw      a5,-20(s0)
        addi    a4,a5,1
        sw      a4,-20(s0)
        sb      zero,0(a5)
        nop
        mv      a0,a5
        lw      ra,300(sp)
        lw      s0,296(sp)
        addi    sp,sp,304
        jr      ra
printf:
        addi    sp,sp,-336
        sw      ra,300(sp)
        sw      s0,296(sp)
        addi    s0,sp,304
        sw      a0,-292(s0)
        sw      a1,4(s0)
        sw      a2,8(s0)
        sw      a3,12(s0)
        sw      a4,16(s0)
        sw      a5,20(s0)
        sw      a6,24(s0)
        sw      a7,28(s0)
                addi    a5,s0,4
        sw      a5,-20(s0)
        addi    a5,s0,-280
        lw      a2,-20(s0)
        lw      a1,-292(s0)
        mv      a0,a5
        call    vsprintf
        sw      a0,-24(s0)
        addi    a5,s0,-280
        mv      a0,a5
        call    write
        lw      a5,-24(s0)
        mv      a0,a5
        lw      ra,300(sp)
        lw      s0,296(sp)
        addi    sp,sp,336
        jr      ra
exit:
        addi    sp,sp,-16
        sw      s0,12(sp)
        addi    s0,sp,16
        li      a7,10
        ecall

        nop
        lw      s0,12(sp)
        addi    sp,sp,16
        jr      ra
.data
.LC0:
        .string "第十项是：%d"
.text
main:
        addi    sp,sp,-32
        sw      ra,28(sp)
        sw      s0,24(sp)
        addi    s0,sp,32
        li      a5,1
        sw      a5,-24(s0)
        li      a5,1
        sw      a5,-28(s0)
        li      a5,3
        sw      a5,-20(s0)
        j       .L26
.L27:
        lw      a4,-24(s0)
        lw      a5,-28(s0)
        add     a5,a4,a5
        sw      a5,-24(s0)
        lw      a5,-24(s0)
        sw      a5,-32(s0)
        lw      a5,-28(s0)
        sw      a5,-24(s0)
        lw      a5,-32(s0)
        sw      a5,-28(s0)
        lw      a5,-20(s0)
        addi    a5,a5,1
        sw      a5,-20(s0)
.L26:
        lw      a4,-20(s0)
        li      a5,10
        ble     a4,a5,.L27
        lw      a1,-28(s0)
        lui     a5,%hi(.LC0)
        addi    a0,a5,%lo(.LC0)
        call    printf
        call    exit
        nop
        lw      ra,28(sp)
        lw      s0,24(sp)
        addi    sp,sp,32
        jr      ra