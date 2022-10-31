# pragma once   // ECALL和startup都要用
#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include "mmu.hpp"
#include "ecall.hpp"
// #include "show.hpp"

namespace CPU {
#define memsize 1000000 //存储器单元数（可修改）
#define memw 8   //一个存储单元8位
#define regnum 32//寄存器个数
#define imsize 10000
#define islui(x) ((x&0b1111111)==(0b110111))
#define isauipc(x) ((x&0b1111111)==0b10111)
#define isjal(x) ((x&0b1111111)==0b1101111)
#define isjalr(x) ((x&0b1111111)==0b1100111)
#define isB(x) ((x&0b1111111)==0b1100011)
#define isL(x) ((x&0b1111111)==3)   //load类
#define isS(x) ((x&0b1111111)==0b0100011)
#define isI(x) ((x&0b1111111)==0b10011)  //立即数类
#define isR(x) ((x&0b1111111)==(0b110011))
#define isf(x) ((x&0b1111111)==0b1111)
#define isec(x) ((x&0b1111111)==0b1110011)
#define ismret(x) ((x)==0x00200073)    // uret/mret
#define getfunct3(x) ((x&((1<<15)-1))>>12)  //得到funct3字段
#define geth7(x) (x>>25)
#define geth12(x) (x>>19)
#define isfence_i(x) (x==0b1000000001111)
#define isfence(x) (x&((15<<28)+((1<<20)-1))==15)
#define isecall(x) (x==0b1110011)
#define isebreak(x) (x==0b1110011+(1<<20))
#define calw(x,w1,w2) ((x>>w1)&((1<<(w2-w1+1))-1))//得到x的第w1到w2位,w2>=w1
#define usemmu 0

char VA[35],buf[35];
bool dm[memsize][memw]; //数据存储器
int im[imsize];  //指令存储器
int reg[regnum]={0,};//32
int pc, mepc;
int ins,pc1,pc2;
int rs1,rs2,rd; //读寄存器结果
/*下为各种控制信号*/
bool use_rs1,use_rs2,imm12_31,imm0_11,imm_jal,imm12_10_5;
bool imm11_5,imm4_1_11,use_shamt,use_imm;
bool usesucc,usepred,usecsr,usezimm;
bool ALUadd,ALUsub,ALUand,ALUor,ALUxor,ALUlm,ALUrm,ALUmul,ALUrem,ALUdiv;
bool link,memrable;
bool memwable,regwable,wimm,immaddpc,nousepc;//记录存储器、寄存器是否可写
bool beqf,bnef,bltf,bgef,bltuf,bgeuf;
bool wALUout;
bool bit,half,word,unsign;/*控制读和写*/
bool slt,sltu;/*这几条算术指令返回的不是算出的结果*/
/*ALUsrc*/
int ALUsrc1,ALUsrc2,ALUout,pos;
bool flag_zero,flag_neg,flag_np,flag_pn;
int memrd,regwd;/*存储器读数、寄存器写入数据*/
bool jump, ecall, mret;

int imm,imms[5];
using namespace std;

void writemem(int address,int data)/*写存储器*/
{
    for(int i=31;i>=0;i--)
        {VA[i]=address&1?'1':'0';
        address>>=1;}
    VA[32]=0;

    for(int i=0, tmp = data;i<4;i++)
        {buf[i]=(tmp & 0xFF);
        tmp>>=8;}
    if(word)    MMU::WriteM(VA,buf,4);
    else if(half) MMU::WriteM(VA,buf,2);
    else if(bit) MMU::WriteM(VA,buf,1);
    
    return;
}
int readmem(int address)/*读存储器*/
{
    for(int i=31;i>=0;i--)
        {VA[i]=address&1?'1':'0';
        address>>=1;}
    VA[32]=0;
    int s;
    if(word)    MMU::ReadM(VA,buf,4),s=4;
    else if(half) MMU::ReadM(VA,buf,2),s=2;
    else if(bit) MMU::ReadM(VA,buf,1),s=1;

    int data=0;
    for(int i=s-1;i>=0;i--){
        data<<=8;
        data|=(unsigned char)buf[i];
    }
    if((!unsign)&&(buf[s-1] & 0x80)){
        data=-(1ll<<(s * 8))+data;
    }

    return data;
}
bool fetch() //取指
{
    word = 1;
    ins=readmem(pc);//读指令
    // printf("0x%08x: 0x%08x\n", pc, ins);
    pc1=pc+4;//pc++
    return 1;
}
bool decode()//译码
{
    /*下为译码部分，实际上相当于17位地址与控制信号的一一映射*/
    // cout<<pc<<"ss"<<ins<<endl;
    use_rs1=0,use_rs2=0,imm12_31=0,imm0_11=0,imm_jal=0;
    imm12_10_5=0,imm11_5=0,imm4_1_11=0,use_imm=0;
    use_shamt=0;usepred=0,usesucc=0,usecsr=0,usezimm=0;
    ALUadd=1,ALUand=0,ALUor=0,ALUsub=0,memwable=0,regwable=0;
    wimm=0,immaddpc=0,link=0,memrable=0,nousepc=0;
    beqf=0,bnef=0,bltf=0,bgef=0,bltuf=0,bgeuf=0;
    bit=0,half=0,word=1,unsign=0;wALUout=0;ALUxor=0;
    slt=0,sltu=0;ALUlm=0;ALUrm=0;ALUmul=0;ALUrem=0,ALUdiv=0;
    jump=0;ecall=0;mret=0;
    if(islui(ins)){
        // cout<<"lui";
        wimm=1;//写立即数
        imm12_31=1;use_imm=1;regwable=1;
    }
    else if(isauipc(ins))
    {
        // cout<<"auipc";
        wimm=1,regwable=1,immaddpc=1;
        imm12_31=1;use_imm=1;
    }
    else if(isjal(ins))
    {
        // cout<<"jal";
        imm_jal=1;use_imm=1;wimm=1;immaddpc=1;
        link=1;jump=1;regwable=1;
    }
    else if(isjalr(ins))
    {
        // cout<<"jalr";
        imm0_11=1;use_imm=1;wimm=1;immaddpc=1;
        link=1;jump=1;regwable=1;nousepc=1;
        if(getfunct3(ins)==0)
            imm0_11=1,use_rs1=1;
        else
        {
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isB(ins))
    {
        // cout<<"B";
        use_rs1=1;
        use_rs2=1;wimm=1;immaddpc=1;
        imm12_10_5=1;
        imm4_1_11=1;
        ALUadd=0,ALUsub=1;
        switch (getfunct3(ins))
        {
        case 0://beq          
            beqf=1;      
            break;
        case 1://bne
            bnef=1;
            break;
        case 4://blt
            bltf=1;
            break;
        case 5://bge
            bgef=1;
            break;
        case 6://bltu
            bltuf=1;
            break;
        case 7://bgeu
            bgeuf=1;
            break;
        default:
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isL(ins))
    {
        // cout<<"L";
        imm0_11=1,use_rs1=1,use_imm=1;
        memrable=1;regwable=1;word=0;
        switch (getfunct3(ins))
        {
        case 0://lb
            bit=1;      
            break;
        case 1://lh
            half=1;
            break;
        case 2://lw
            word=1;
            break;
        case 4://lbu
            bit=1;
            unsign=1;
            break;
        case 5://lhu
            half=1;
            unsign=1;
            break;
        default:
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isS(ins))
    {
        // cout<<"s";
        imm11_5=1;use_rs1=1,use_rs2=1,use_imm=1;
        memwable=1;word=0;
        switch (getfunct3(ins))
        {
        case 0://sb
            bit=1;      
            break;
        case 1://sh
            half=1;
            break;
        case 2://sw
            word=1;
            break;
        default:
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isI(ins))
    {
        // cout<<"I";
        use_rs1=1,imm0_11=1,use_imm=1;
        ALUadd=0;regwable=1;wALUout=1;
        switch (getfunct3(ins))
        {
        case 0://addi  
            ALUadd=1;     
            break;
        case 2://slti
            ALUsub=1;
            slt=1;
            break;
        case 3://sltiu
            ALUsub=1;
            sltu=1;
            break;
        case 4://xori
            ALUxor=1;
            break;
        case 6://ori
            ALUor=1;
            break;
        case 7://andi
            ALUand=1;
            break;
        case 1:
            imm0_11=1,use_imm=1;
            if(!geth7(ins))//slli
            {   
                ALUlm=1;
                printf("Invalid instruction!");
                exit(0);
            }
            use_shamt=1;
            break;
        case 5:
            imm0_11=1,use_imm=1;
            if(!geth7(ins)){//srli
                ALUrm=1;
                unsign=1;
                use_shamt=1;
            }
            else if(geth7(ins)==(1<<5)){//srai        
                ALUrm=1;     
                use_shamt=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            
            break;
        default:
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isR(ins))
    {
        // cout<<"R";
        use_rs1=1,use_rs2=1;regwable=1;wALUout=1;ALUadd=0;
        switch (getfunct3(ins))
        {
        case 0:
            if(geth7(ins)==0){//add 
                ALUadd=1;
            }
            else if(geth7(ins)==32){//sub
                ALUsub=1;
            }
            else if (geth7(ins)==1){//mul
                ALUmul=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }       
            break;
        case 1:
            if(geth7(ins)==0){//sll
            }
            else{
                ALUlm=1;
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        case 2:
            if(geth7(ins)==0){//slt
                ALUsub=1;
                slt=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;

        case 3:
            if(geth7(ins)==0){//sltu
                ALUsub=1;
                sltu=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        case 4:
            if(geth7(ins)==0){//xor
                ALUxor=1;
            }
            else if(geth7(ins)==1){//div
                ALUdiv=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        case 5:
            if(geth7(ins)==0){//srl
                ALUrm=1;
                unsign=1;
            }
            else if(geth7(ins)==32){//sra
                ALUrm=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        case 6:
            if(geth7(ins)==0){//or
                ALUor=1;
            }
            else if(geth7(ins)==1){//rem
                ALUrem=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        case 7:
            if(geth7(ins)==0){//and
                ALUand=1;
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;
        default:
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if(isf(ins))/*没有实现*/
    {
        // cout<<"f";
        if(isfence(ins))//fence
        {
            usepred=1,usesucc=1;
        }
        else if(isfence_i(ins)){//fence.i

        }
        else{
            printf("Invalid instruction!");
            exit(0);
        }
    }
    else if (ismret(ins)) {//放在isec前面
        mret = 1;
    }
    else if(isec(ins))
    {
        // cout<<"ec";
        switch (getfunct3(ins))
        {
        case 0:
            if(isecall(ins)){//ecall   /*没有实现*/
                mepc = pc1;
                ECALL::ecall_control(reg[17], &pc);
                ecall=1;
            }
            else if(isebreak(ins)){//ebreak  /*交给更高层控制*/
                // cout<<pc<<" "<<reg[2]<<" "<<reg[3]<<" "<<reg[4];
                printf("1 to exit,2 to continue:\n");
                int x;
                scanf("%d",&x);
                if(x==1){
                    exit(0);
                }
                else if(x==2)
                    ;
                else{
                    printf("Error!");
                    exit(0);
                }
            }
            else{
                printf("Invalid instruction!");
                exit(0);
            }
            break;

        /*下面的还没实现*/
        case 1://csrrw
            use_rs1=1;
            usecsr=1;
            break;
        case 2://csrrs
            use_rs1=1;
            usecsr=1;
            break;
        case 3://csrrc
            use_rs1=1;
            usecsr=1;
            break;
        case 5://csrrwi
            usecsr=1;
            usezimm=1;
            break;
        case 6://cssrrsi
            usezimm=1;
            usecsr=1;
            break;
        case 7://csrrci
            usezimm=1;
            usecsr=1;
            break;
        default:
            break;
        }
        
    }
    else
    {
        // cout<<pc<<endl;
        printf("Invalid Instruction!");
        exit(0);
    }

    /*读寄存器*/
    rd=(ins&((1<<12)-1))>>7;
    // cout<<rd<<"tt";
    rs1=reg[(ins&((1<<20)-1))>>15];
    rs2=reg[(ins&((1<<25)-1))>>20];
    // cout<<((ins&((1<<25)-1))>>20)<<" rs2"<<rs2<<"\n";
    /*计算立即数*/
    /*12_31*/
    imms[0]=calw(ins,12,31)<<12;
    /*20,1_10,11,12_19*/
    imms[1]=(calw(ins,31,31)<<20)+(calw(ins,21,30)<<1)+(calw(ins,20,20)<<11)+(calw(ins,12,19)<<12);
    /*0_11*/
    imms[2]=calw(ins,20,31)&(1<<11)?(-(1<<12)+calw(ins,20,31)):calw(ins,20,31);
    /*12,5_10,1_4,11*/
    imms[3]=(calw(ins,31,31)<<12)+(calw(ins,25,30)<<5)+((calw(ins,8,11)<<1)+(calw(ins,7,7)<<11));
    if(imms[3]&(1<<12))
        imms[3]=-(1<<13)+imms[3];
    /*5_11,0_4*/
    imms[4]=calw(ins,7,11)+(calw(ins,25,31)<<5);
    if(imms[4]&(1<<11))
        imms[4]=-(1<<12)+imms[4];

    return 1;
}   
bool execute()//执行
{
    /*得到立即数*/
    if(imm12_31)
        imm=imms[0];
    else if(imm_jal)
        imm=((imms[1]&(1<<20))?(-(1<<21)+imms[1]):imms[1]);
    else if(imm0_11)
        imm=imms[2];
    else if(imm12_10_5&&imm4_1_11)
        imm=imms[3];
    else if(imm11_5)
        imm=imms[4];
    else
        imm=0;
    /*计算pc2*/
    
    if(nousepc)
        pc2=imm+rs1;
    else
        pc2=pc+imm;
    // cout<<nousepc<<"a"<<" "<<pc<< " "<<imm;
    /*额外控制，这里的imm其实是写入寄存器的指令地址*/
    if(immaddpc)/*auipc*/
        imm+=pc;
    if(link)/*jal*/
        imm=pc1;
    /*得到ALUsrc*/
    if(use_imm){
        ALUsrc2=imm;
        ALUsrc1=rs1;
    }
    else{
        ALUsrc2=rs2;
        ALUsrc1=rs1;
    }
    /*ALU计算*/
    if(ALUadd){
        ALUout=ALUsrc1+ALUsrc2;
    }
    else if(ALUsub){
        ALUout=ALUsrc1-ALUsrc2;
    }
    else if(ALUand){
        ALUout=ALUsrc1&ALUsrc2;
    }
    else if(ALUor){
        ALUout=ALUsrc1|ALUsrc2;
    }
    else if(ALUxor){
        ALUout=ALUsrc1^ALUsrc2;
    }
    else if(ALUlm){
        ALUout=ALUsrc1<<ALUsrc2;
    }
    else if(ALUrm){
        bool flag=ALUsrc1<0;
        
        if(unsign){
            ALUout=((unsigned int)ALUsrc1)>>ALUsrc2;
        }
        else{
            ALUout=ALUsrc1>>ALUsrc2;
        }
    }
    else if (ALUmul) {
        ALUout = ALUsrc1 * ALUsrc2;
    }
    else if (ALUdiv) {
        if (ALUsrc2 == 0) {
            printf("Divided by zero.\n");
            return 0;
        }
        ALUout = ALUsrc1 / ALUsrc2;
    }
    else if (ALUrem) {
        if (ALUsrc2 == 0) {
            printf("Divided by zero.\n");
            return 0;
        }
        ALUout = ALUsrc1 % ALUsrc2;
    }
    else{
        printf("ALU func error!\n");
        return 0;
    }
    /*ALU flag*/
    flag_zero=ALUout==0;
    flag_neg=ALUout<0;
    flag_np=(ALUsrc1<0) & (ALUsrc2>=0);
    flag_pn=(ALUsrc1>=0) & (ALUsrc2<0);

    pos=ALUout;
    return 1;
}
bool memory()//访存
{
    /*存储器*/
    if(memwable){
        writemem(pos,rs2);
    }
    else if(memrable)
        memrd=readmem(pos);
    /*jump控制*/
    if(beqf) jump=flag_zero==1;
    else if(bnef) jump=flag_zero==0;
    else if(bltf) jump=flag_neg==1;
    else if(bltuf){
        if(flag_np) jump=0;
        else if(flag_pn) jump=1;
        else jump=flag_neg==1;
    }
    else if(bgef) jump=!(flag_neg);
    else if(bgeuf){
        if(flag_np) jump=1;
        else if(flag_pn) jump=0;
        else jump=!(flag_neg);
    }
    /*ALUout处理*/
    if(slt){
        ALUout=flag_neg;
    }
    else if(sltu){
        if(flag_np) jump=0;
        else if(flag_pn) jump=1;
        else jump=(flag_neg);
    }
    return 1;
}
bool writeback()//写回
{
    /*得到regwd*/
    if(wimm)
        regwd=imm;
    else if(wALUout)
        regwd=ALUout;
    else
        regwd=memrd;
    /*writeback*/
    if(regwable)
    {
        // cout<<"Ss"<<rd<<" "<<regwd<<" "<<ALUout<<endl;
        if (rd!=0)
            reg[rd]=regwd;
    }
    /*getnextpc*/
    if(jump)
        pc=pc2;
    else if(mret) {
        pc = mepc;
        SHOW::show();
    }
    else if (!ecall)
        pc=pc1;

    return 1;
}

void cycle()
{
    while(1)
    {
        if (pc==0x100020)
            pc = pc;
        if(!fetch())
            break;
        if(!decode())
            break;
        if(!execute())
            break;
        if(!memory())
            break;
        if(!writeback())
            break;
    }
    printf("finished");
    return;
}
void init() {
    reg[2] = 0x7FFFEFFC;
    MMU::init();
    ECALL::init();
    SHOW::init();
}
// int main()
// {
//     init();
//     cycle();
// }
}