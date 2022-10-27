#include "page.h"
#include <math.h>
//#include "mymalloc.h"


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
                tb->entry[i].valid = '0';
                for(j=0;j<11;j++)
                {
                        tb->entry[i].Control[j]='0';
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
                tb->entry[i].valid = '0';
                tb->entry[i].dirty = '0';
                for(j=0;j<10;j++)
                {
                        tb->entry[i].Control[j]='0';
                }
                tb->entry[i].Base = 0;

        }
}