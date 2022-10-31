#pragma once
#include <math.h>
//#include "mymalloc.h"
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
/*明天彻底改掉，页表的空间单独开，不要和数据混为一谈，太麻烦了*/

void Init_First(void* TB) // 一级页表初始化
{
        int i,j;
        if(TB==NULL)
        {
                printf("First-TB Malloc Error!!\n");
                exit(0);
        }
        struct FirstPgTBL*tb = (struct FirstPgTBL*)TB;
        //tb->entry = (DirEntry*)malloc(TABLE_SIZE*(sizeof(SecondPgTBL*)+12*sizeof(char)));
        
        tb->size = TABLE_SIZE;
        for(i=0;i<TABLE_SIZE;i++)
        {
                tb->entry[i].valid = 0;
                for(j=0;j<11;j++)
                {
                        tb->entry[i].Control[j]=0;
                }
                tb->entry[i].base = NULL;

        }
}
void Init_Second(void* TB) //二级页表初始化
{
        int i,j;
        if(TB==NULL)
        {
                printf("Second-TB Malloc Error!!\n");
                exit(0);
        }
        struct SecondPgTBL*tb = (struct SecondPgTBL*)TB;
        //tb->entry = (PgEntry*)malloc(TABLE_SIZE*(sizeof(int)+12*sizeof(char)));
        tb->size = TABLE_SIZE;
        for(i=0;i<TABLE_SIZE;i++)
        {
                tb->entry[i].valid = 0;
                tb->entry[i].rdonly = 0;
                tb->entry[i].dirty = 0;
                for(j=0;j<10;j++)
                {
                        tb->entry[i].Control[j]=0;
                }
                tb->entry[i].Base = -1;

        }
}