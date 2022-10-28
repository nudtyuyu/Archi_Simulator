/*
*Author: Healer
*Date: Oct. 2022
*Description: This file simulates the work of MMU
*/

#pragma comment(linker, "/STACK:102400000,102400000")
#include <stdio.h>
#include <stdint.h>
#include "page.h"
#include  "Memory.h"
#include <string.h>
#include <math.h>

using namespace std;

#define DIR_LENGTH 10
#define PAGE_LENGTH 10
#define OFFSET_LENGTH 12

char Mem[MEMSIZE]={0};  //主存16MB
char Busy[MEMSIZE/PAGE_SIZE]={0};
char *Disk[DISKSIZE];
char Map[MAPSIZE];
unsigned long start_code = 0;

void do_no_page();
void Init_Disk(char**Disk);
void Init_Map(char*Map);
int get_empty_page();
int get_free_page();
void bread_page(int page,char*M,int file,int n);
void oom();
int b2d(char *num,int n);
int WriteMem(char*buf,int addr,int n);
int ReadMem(char*buf,int addr,int n);
int bmap(int block);

void Init_Map(char*Map)
{
        int i;
        for(i=0;i<MAPSIZE;i++)
        {
                Map[i] = -1;
        }
}


void oom()
{
        printf("Critical Error: Out of Memory!\n");
        exit(0);
}

int bmap(int block,char*Map) //逻辑块号映射到磁盘块号，不涉及多个进程，就返回简单的映射即可。
{
        return block;
}

void Init_Disk(char**Disk)
{
        int i;
        for(i=0;i<DISKSIZE;i++)
        {
                Disk[i] = (char*)malloc(DISKBLK*sizeof(char));
                /* for (size_t j = 0; j < DISKBLK; j++)
                {
                        Disk[i][j] = 0;
                } */
                
        }
}

int WriteMem(char*buf,int addr,int n)
{
        int i;
        int j = 0;
        for(i=addr;i<n+addr;i++)
        {
                Mem[i] = buf[j];
                j++;
        }
        return n;
}

void bread_page(int page,char*M,char**Disk,int* nb)
{
        //FILE*file;
        //file = fopen(Disk[nb],"r");
        char*buf = (char*)malloc(sizeof(char)*PAGE_SIZE);
        int cnt;
        int addr = page*PAGE_SIZE;
        int i,j;
        for(j=0;j<PAGE_SIZE/DISKBLK;j++)
        {
                for(i=0;i<DISKBLK;i++)
                {
                        buf[i] = Disk[nb[j]][i];
                }
                cnt = WriteMem(buf,addr,DISKBLK);
                printf("buf: %d\n",buf[6]);
                printf("%d\t%d\n",cnt,addr);
                addr+=cnt;
        }
        return;
}

int get_empty_page()
{
        int i;
        int pageNum = -1;
        for(i=0;i<MEMSIZE/PAGE_SIZE;i++)
        {
                if(Busy[i]==0)
                {
                        pageNum = i;
                        Busy[i] = 1;
                        break;
                }

        }
        if(pageNum==-1)
        {
                oom();
                //exit(0);
        }
        return pageNum;
}


int get_free_page()
{
        int i;
        int pageNum = -1;
        for(i=0;i<MEMSIZE/PAGE_SIZE;i++)
        {
                if(Busy[i]==0)
                {
                        pageNum = i;
                        Busy[i] = 1;
                        break;
                }

        }
        if(pageNum==-1)
        {
                oom();
                //exit(0);
        }
        return pageNum;
        
}
void do_no_page(char*VA,void*M,unsigned long error_code)
{
        struct FirstPgTBL* TBL1 = (struct FirstPgTBL*) M;
        int32_t address = b2d(VA,32);
        address &= 0xfffff000; //逻辑页号
        unsigned long tmp_add = address - start_code;
        int block = tmp_add/DISKBLK; //计算该地址在第几块逻辑块号。

        char* Addr1 = (char*)malloc(sizeof(char)*(DIR_LENGTH+1)); //一级页表内偏移
        char* Addr2 = (char*)malloc(sizeof(char)*(PAGE_LENGTH+1));//二级页表内偏移
        char* Offset = (char*)malloc(sizeof(char)*(OFFSET_LENGTH+1));//页内偏移
        //复制，转成数字int
        strncpy(Addr1,VA,DIR_LENGTH);
        strncpy(Addr2,VA+DIR_LENGTH,PAGE_LENGTH);
        strncpy(Offset,VA+DIR_LENGTH+PAGE_LENGTH,OFFSET_LENGTH);
        //struct Memory* Mem = (struct Memory*)M;
        //struct FirstPgTBL* TBL1 = (struct FirstPgTBL*) M;
        int32_t addr1 = b2d(Addr1,10);
        int32_t addr2 = b2d(Addr2,10);
        int32_t offset = b2d(Offset,12);
        
        //初始化一个二级页表
        SecondPgTBL*tmp = (SecondPgTBL*)malloc(TABLE_SIZE*(sizeof(PgEntry))+sizeof(int));
        Init_Second(tmp);
        //struct DirEntry* dir = &(TBL1->entry[addr1]);
        //struct DirEntry* dir = &(TBL1->entry[addr1]);
        int nr[4];
        for (int i=0 ; i<4 ; block++,i++)
                nr[i] = bmap(block,Map);
        int page;
        //int dev,n; //暂时乱写的，用于磁盘和磁盘地址(没用了)
        switch (error_code)
        {
                case 0:
                        /*这是一级页表出现页故障，且是换入已有的页*/
                        //dir->base = tmp;
                        TBL1->entry[addr1].base = tmp;
                        page = get_free_page();
                        bread_page(page,Mem,Disk,nr);
                        tmp->entry[addr2].Base = page;
                        TBL1->entry[addr1].valid = 1;
                        tmp->entry[addr2].valid = 1;
                        break;
                case 1:
                        /*这是二级页表出现页故障或者load on demand*/
                        free(tmp);
                        //int page;
                        if(TBL1->entry[addr1].base->entry[addr2].Base!=-1)
                        {
                                bread_page(page,Mem,Disk,nr);
                                TBL1->entry[addr1].base->entry[addr2].valid = 1;
                                return;
                        }
                        page = get_free_page();
                        bread_page(page,Mem,Disk,nr);
                        TBL1->entry[addr1].base->entry[addr2].valid = 1;
                        TBL1->entry[addr1].base->entry[addr2].Base = page;
                        break;
                
                case 3:
                        /*这是零页*/
                        TBL1->entry[addr1].base = tmp;
                        page = get_empty_page();
                        tmp->entry[addr2].Base = page;
                        TBL1->entry[addr1].valid = 1;
                        tmp->entry[addr2].valid = 0;
                        break;
                default:
                        break;
        }
        return;
}

int b2d(char* num_2,int n)
{//2进制字符串转化为10进制整数 

        int k=0;

        int num_10=0;//10进制数 

        for(int i=n-1;i>=0;i--){

                num_10+=(num_2[i]-'0')*pow(2,k);//按权展开，累加

                k++;//权值递增 

        }

        return num_10;//返回十进数 

}

int VA2PA(char* VA,void*M,int base)
{
        char* Addr1 = (char*)malloc(sizeof(char)*(DIR_LENGTH+1)); //一级页表内偏移，+1位用来放置\0
        char* Addr2 = (char*)malloc(sizeof(char)*(PAGE_LENGTH+1));
        char* Offset = (char*)malloc(sizeof(char)*(OFFSET_LENGTH+1));
        /*明天接着写，后面就是在一级页表里面找，如果有效位为1，进入二级页表，否则触发页故障
         *进入二级页表之后，通过二级页表的偏移找到对应的地址，若有效位为1，返回物理地址，否则触发页故障，
        */
       strncpy(Addr1,VA,DIR_LENGTH);
       strncpy(Addr2,VA+DIR_LENGTH,PAGE_LENGTH);
       strncpy(Offset,VA+DIR_LENGTH+PAGE_LENGTH,OFFSET_LENGTH);
       //struct Memory* Mem = (struct Memory*)M;
       struct FirstPgTBL* TBL1 = (struct FirstPgTBL*) M;
       int32_t addr1 = b2d(Addr1,10);
       int32_t addr2 = b2d(Addr2,10);
       int32_t offset = b2d(Offset,12);
       struct DirEntry *dir = &(TBL1->entry[addr1]);
       if(dir->valid==0)
       {
                do_no_page(VA,M,0);
       }
       struct SecondPgTBL* base2 = dir->base;
       if(base2->entry[addr2].valid==0)
       {
                do_no_page(VA,M,1);
       }
       int32_t PA_Base = (base2->entry[addr2].Base<<12) & 0xFFFFF000;
       //printf("%d\t%d\n",PA_Base,base2->entry[addr2].Base);
       int PA = PA_Base | offset;
       return PA;
       /* strncpy(base2,dir.Base,20);
       for(int i = 20;i<WORD;i++)
       {
                base2[i] = '0';
       } */
       //printf("The value is:%d\n",base2->entry[addr2].Base);
}



int main()
{
        char VA[33] = "00000000000000000000000000000110";
        //char start[33] = "00000000010000000000000000000000";
        //start_code = b2d(start,32);
        //char PA[33];
        struct FirstPgTBL* TBL1 = (FirstPgTBL*)malloc(TABLE_SIZE*(sizeof(DirEntry))+sizeof(int));
        struct SecondPgTBL* TBL2 = (SecondPgTBL*)malloc(TABLE_SIZE*(sizeof(PgEntry))+sizeof(int));
        //struct FirstPgTBL *TBL1;
        //struct SecondPgTBL*TBL2;
        /* Map[0] = 2;
        Map[1] = 3;
        Map[2] = 4;
        Map[3] = 5; */
        Init_First(TBL1); //初始化一级页表
        Init_Disk(Disk);//初始化Disk
        Disk[0][6] = 46;
        

        //Init_Second(TBL2);//初始化二级页表

        //strncpy(TBL1->entry[3].Base,TBL2,10);
        //随便改改一级页表和二级页表的页表项
        //TBL1->entry[3].base = TBL2;
        //TBL2->entry[3].Base = 1;
        //虚实地址转换
        int32_t PA = VA2PA(VA,TBL1,0);
        printf("The Physical Address is: %d, The data is: %d",PA,Mem[PA]);
        return 0;
}