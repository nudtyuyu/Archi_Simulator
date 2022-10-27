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

void do_no_page();

void do_no_page()
{
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
       struct DirEntry dir = TBL1->entry[addr1];
       if(!dir.valid)
       {
                do_no_page();
       }
       struct SecondPgTBL* base2 = dir.base;
       int32_t PA_Base = (base2->entry[addr2].Base<<12) & 0xFFFFF000;
       printf("%d\t%d\n",PA_Base,base2->entry[addr2].Base);
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
        char VA[33] = "00000000110000000011000000000000";
        //char PA[33];
        struct FirstPgTBL* TBL1 = (FirstPgTBL*)malloc(TABLE_SIZE*(sizeof(DirEntry))+sizeof(int));
        struct SecondPgTBL* TBL2 = (SecondPgTBL*)malloc(TABLE_SIZE*(sizeof(PgEntry))+sizeof(int));
        //struct FirstPgTBL *TBL1;
        //struct SecondPgTBL*TBL2;
        Init_First(TBL1);
        Init_Second(TBL2);

        //strncpy(TBL1->entry[3].Base,TBL2,10);
        TBL1->entry[3].base = TBL2;
        TBL2->entry[3].Base = 1;
        int32_t PA = VA2PA(VA,TBL1,0);
        printf("The Physical Address is: %d\n",PA);
        return 0;
}