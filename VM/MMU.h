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

void do_no_page(char*VA,void*M,unsigned long error_code);
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
int ReadM(char*VA,char*buf,void*tlb,int n); //用户态读内存
int WriteM(char*VA,char*buf,void*tlb,int n); //用户态写内存
void SetTLB(MyTLB*TLB,PgEntry *entry,char*VA);
//void TLBWriteBack();
int32_t TLBRead(MyTLB* TLB,char*VA);
int32_t VA2PA(char* VA,void*M,void*tlb,int op);
int ChangePage();//页替换策略，随机法替换。用不着了