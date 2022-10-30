#include "ecall.h"



void ecall_control(int code,...)
{
        va_list ap; //可变参数列表
        
        if(code==0)
        {
                
                va_start(ap,code);//ap指向参数中的第一个参数,code是最后一个有名参数
                char *VA = va_arg(ap,char*);//获取第一个可变参数的值，并指向下一个参数
                void* M  = va_arg(ap,void*);
                int error_code = va_arg(ap,int);
                va_end(ap);//结束
                do_no_page(VA,M,error_code);
                return;

        }
        else
                ;
        return  ;
}