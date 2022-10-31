#pragma once
#include <cstdarg>
#include "mmu.hpp"
#include "show.hpp"
namespace ECALL {
    int syscall_table[64]; // 散转表
    void init() {
        syscall_table[4] = 0x100000;
        syscall_table[10] = 0x10004c;
    }
    void ecall_control(int code, ...) {
        va_list ap;         //可变参数列表
        va_start(ap, code); // ap指向参数中的第一个参数,code是最后一个有名参数

        if (code == 0) // 页故障
        {
            char *VA = va_arg(ap, char *); //获取第一个可变参数的值，并指向下一个参数
            int error_code = va_arg(ap, int);
            va_end(ap); //结束
            MMU::do_no_page(VA, error_code);
            return;
        } else if (code == 4) { // write
            int *pc = va_arg(ap, int *);
            *pc = syscall_table[4]; // sys_write
            // printf("ecall write\n");
        } else if (code == 10) {    // exit
            int *pc = va_arg(ap, int *);
            *pc = syscall_table[10]; // sys_exit
            // printf("0x%08x ecall exit\n", *pc);
        }
        return;
    }
} // namespace ECALL