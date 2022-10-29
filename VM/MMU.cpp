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
#include <time.h>

using namespace std;

#define DIR_LENGTH 10
#define PAGE_LENGTH 10
#define OFFSET_LENGTH 12


char Mem[MEMSIZE]={0};  //主存16MB
char Busy[MEMSIZE/PAGE_SIZE]={0};
char *Disk[DISKSIZE];
char Map[MAPSIZE];
struct MyTLB * TLB;
unsigned long start_code = 0;

void do_no_page();
void Init_Disk(char**Disk); //初始化磁盘
void Init_Map(char*Map); // 初始化磁盘映射表
void Init_TLB(struct MyTLB*TLB); //初始化快表
int get_empty_page(); //分配空页（零页）
int get_free_page(); //获取空闲页
void bread_page(int page,char*M,int file,int n); //从磁盘按块读取数据
void oom(); //out of Memory
int b2d(char *num,int n); // 将二进制字符串转为10进制int
int WriteMem(char*Mem,char*buf,int addr,int n); //内核态写内存
int ReadMem(char*M,int addr,int n,char*buf); //内核态读内存
//int ReadMem(char*buf,int addr,int n);
int bmap(int block); //映射，文件相对块->磁盘块号
int ReadM(char*VA,char*buf,int n); //用户态读内存
int WriteM(char*VA,char*buf,int n); //用户态写内存
void SetTLB(MyTLB*TLB,PgEntry *entry,char*VA);
//void TLBWriteBack();
int32_t TLBRead(MyTLB* TLB,char*VA);
int32_t VA2PA(char* VA,void*M,void*tlb,int op);
int ChangePage();//页替换策略，随机法替换。

void SetTLB(MyTLB *TLB,PgEntry *entry,char*VA)
{
        int i;
        int index =-1;
        for(i=0;i<TLBSIZE;i++)
        {
                if(TLB->valid[i]==0 || TLB->entry[i].dirty ==1) //空闲页或者脏记录
                {
                        TLB->entry[i].Base = entry->Base;
                        for(int j = 0;j<9;j++)
                        {
                                TLB->entry[i].Control[j] = entry->Control[j];
                        }   
                        TLB->entry[i].dirty = entry->dirty;
                        TLB->entry[i].rdonly = entry->rdonly;
                        TLB->entry[i].valid = entry->valid;
                        int x = 6;
                        for(int k=0;k<26;k++)
                        {
                                TLB->entry[i].tag[k] = VA[x];
                                x++;
                        }
                        index = i;
                        break;
                }
        }
        if(index == -1) //页替换策略：FIFO
        {
                i = 0;
                
                TLB->entry[i].Base = entry->Base;
                for(int j = 0;j<9;j++)
                {
                        TLB->entry[i].Control[j] = entry->Control[j];
                }   
                int x = 6;
                for(int k=0;k<26;k++)
                {
                        TLB->entry[i].tag[k] = VA[x];
                        x++;
                }
                TLB->entry[i].dirty = entry->dirty;
                TLB->entry[i].rdonly = entry->rdonly;
                TLB->entry[i].valid = entry->valid;
                index = i;
        }

}

int32_t TLBRead(MyTLB* TLB,char*VA)
{
        int i;
        int32_t base;
        for(i=0;i<TLBSIZE;i++)
        {
                if(!strncmp(TLB->entry[i].tag,VA+6,26))
                {
                        if(TLB->valid[i]==1)
                        {
                                base = TLB->entry[i].Base;
                                return base;
                        }
                }
        }
        return -1;
}

void Init_TLB(struct MyTLB*TLB) //初始化快表
{
        
        int i,j;
        struct MyTLB*tb = TLB;
        //tb->entry = (PgEntry*)malloc(TABLE_SIZE*(sizeof(int)+12*sizeof(char)));
        for(i=0;i<TLBSIZE;i++)
        {
                tb->entry[i].valid = 0;
                tb->entry[i].rdonly = 0;
                tb->entry[i].dirty = 0;
                for(j=0;j<10;j++)
                {
                        tb->entry[i].Control[j]=0;
                }
                for(j =0;j<26;j++)
                {
                        tb->entry[i].tag[j] = 0;
                }
                tb->entry[i].Base = -1;

        }
        for(i=0;i<TLBSIZE;i++)
        {
                tb->valid[i] = 0;
        }
}

int ReadM(char*VA,char*buf,void*tlb,int n) //用户态的读内存
{
        FirstPgTBL* TBL1;
        MyTLB* TLB = (MyTLB*)tlb;
        int32_t PA = VA2PA(VA,TBL1,TLB,0);
        int32_t i,j=0;
        if(buf==NULL)
        {
                buf = (char*)malloc(sizeof(char)*(n+1));
                buf[n] = 0;
        }
        for(i = PA;i<PA+n;i++)
        {
                buf[j] = Mem[i];
                j++;
        }
        return n;
}

int WriteM(char*VA,char*buf,void*tlb,int n) //用户态的读内存
{
        FirstPgTBL* TBL1; //这个不对，到时候传全局变量TBL1
        MyTLB* TLB = (MyTLB*)tlb;
        int32_t PA = VA2PA(VA,TBL1,TLB,0);
        int32_t i,j=0;
        if(buf==NULL)
        {
                printf("Error! WriteMem Buffer is NULL!\n");
                exit(0);
        }
        for(i = PA;i<PA+n;i++)
        {
                Mem[i] = buf[j];
                j++;
        }
        return n;
}


/*明天（10.29）实现TLB和替换策略*/
/*待会儿实现用户态WriteMem和ReadMem(√)*/
/*还没有实现rdonly的一个判断问题，记得实现*/







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

int ReadMem(char*M,int addr,int n,char*buf) //内核态使用的readMem
{
        int i,j=0;
        if(buf==NULL)
        {
                buf = (char*)malloc(sizeof(char)*(n+1));
                buf[n] = 0;
        }
        for(i=addr;i<addr+n;i++)
        {
                buf[j] = M[i];
                j++;
        }
        //buf[j] = 0;
        return n;
}

int WriteMem(char* Mem,char*buf,int addr,int n)  //内核态使用的WriteMem
{
        int i;
        int j = 0;
        if(buf==NULL)
        {
                printf("Error! WriteMem Buffer is NULL!\n");
                exit(0);
        }
        for(i=addr;i<n+addr;i++)
        {
                Mem[i] = buf[j];
                j++;
        }
        return n;
}

void bread_page(int page,char*M,char**Disk,int* nb) //将磁盘中的内容读到新分配的页中
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
                cnt = WriteMem(Mem,buf,addr,DISKBLK);
                printf("buf: %d\n",buf[6]);
                printf("%d\t%d\n",cnt,addr);
                addr+=cnt;
        }
        return;
}

int get_empty_page() //找一个空闲页，全部置0
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
        for(i = pageNum*PAGE_SIZE;i<pageNum*PAGE_SIZE+PAGE_SIZE;i++)
        {
                Mem[i] = 0;
        }
        return pageNum;
}

int ChangePage()
{
        srand((unsigned int)time(NULL));
        int index = 0 + rand() % ( MEMSIZE/PAGE_SIZE -0 +1 );
        return index;
        
}

int get_free_page() //找一个空闲页
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
                
                //return -1; //鉴于页面替换真的复杂的有点离谱，算了。反正我的磁盘大小设计的和页面数是匹配的......
                oom();
                //exit(0);
        }
        return pageNum;
        
}
void do_no_page(char*VA,void*M,unsigned long error_code) //缺页处理
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
                        /* if(page==-1)
                        {
                                int num = ChangePage();// 页替换策略
                                
                        } */
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

int32_t VA2PA(char* VA,void*M,void*tlb,int op) //虚地址映射到实地址,op表示虚实地址转换涉及的操作（读还是写）
{
        //op = 0 读该地址，op = 1 写该地址
        
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
       struct MyTLB* TLB = (struct MyTLB*) tlb;
       int32_t addr1 = b2d(Addr1,10);
       int32_t addr2 = b2d(Addr2,10);
       int32_t offset = b2d(Offset,12);

       int32_t BASE =TLBRead(TLB,VA);
       if(BASE != -1 )
       {
                BASE  = (BASE<<(12)) &0xFFFFF000;
                int32_t result = BASE | offset;
                return result;
       }
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
       if(op==1)
       {
                if(base2->entry[addr2].rdonly)
                {
                        printf("This address is not readable!!\n"); //因为不涉及fork进程，暂时不实现do_wp_page()
                        exit(0);
                }
       }
       SetTLB(TLB,&base2->entry[addr2],VA);
       int32_t PA_Base = (base2->entry[addr2].Base<<12) & 0xFFFFF000;
       //printf("%d\t%d\n",PA_Base,base2->entry[addr2].Base);
       int32_t PA = PA_Base | offset;
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
        TLB = (struct MyTLB*)malloc(TLBSIZE*(sizeof(TLBEntry)+sizeof(char)));
        Init_TLB(TLB);
        Disk[0][6] = 46;
        

        //Init_Second(TBL2);//初始化二级页表

        //strncpy(TBL1->entry[3].Base,TBL2,10);
        //随便改改一级页表和二级页表的页表项
        //TBL1->entry[3].base = TBL2;
        //TBL2->entry[3].Base = 1;
        //虚实地址转换
        int32_t PA = VA2PA(VA,TBL1,TLB,0);
        printf("The Physical Address is: %d, The data is: %d",PA,Mem[PA]);
        return 0;
}