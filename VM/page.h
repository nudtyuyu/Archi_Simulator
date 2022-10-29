#include <stdio.h>
#include <string>
using namespace std;


#define TABLE_SIZE 1024
#define WORD 32
#define PAGE_SIZE 4096
#define TLBSIZE 20
struct PgEntry //二级页表页表项
{
        /* data */
        //char Base[20]; 
        int32_t Base;//物理地址，页面基址
        char Control[9]; //其他控制位，在这里不关心，可扩展
        char rdonly; //该页只读
        char dirty;//脏位，表示最近该页被修改过
        char valid;//有效位，表示页面有效
};

struct SecondPgTBL //二级页表
{
        /* data */
        struct PgEntry entry[TABLE_SIZE];  //二级页表的页表项
        int32_t size;//二级页表的大小
        
};

struct DirEntry //一级页表页表项
{
        /* data */
        //char Base[20]; //下一级页表的基址
        SecondPgTBL* base;
        char Control[11]; //其他控制位，在这里不关心，可扩展
        char valid;//有效位，表示二级页表有效
};


struct FirstPgTBL //一级页表
{
        /* data */
        struct DirEntry entry[TABLE_SIZE];  //一级页表的页表项
        int32_t size;//一级页表的大小
        
};


struct TLBEntry
{
        /* data */
        //char Base[20]; 
        int32_t Base;//物理地址，页面基址
        char Control[9]; //其他控制位，在这里不关心，可扩展
        char rdonly; //该页只读
        char dirty;//脏位，表示最近该页被修改过
        char valid;//有效位，表示页面有效
        char tag[26];
};



struct MyTLB
{
        /* data */
        //char tag[26];
        struct TLBEntry entry[TLBSIZE];  //二级页表的页表项
        char valid[TLBSIZE];//快表的有效位
};










void Init_First(void* TB);
void Init_Second(void* TB);