sys_write:
    addi    sp,sp,-12
    sw      a1,4(sp)
    sw      a2,8(sp)
    sw      a3,12(sp)
    addi    a3,x0,0
.here:
    lb      a1,0(a0)
    beq     a1,x0,.return
    lui     a2,0x10000  # 显存起始的高20位
    add     a2,a2,a3
    sb      a1,0(a2)
    addi    a3,a3,1
    add     a0,a0,1
    j       .here
.return:
    add     a0,x0,a3
    lw      a1,4(sp)
    lw      a2,8(sp)
    lw      a3,12(sp)
    j       ra