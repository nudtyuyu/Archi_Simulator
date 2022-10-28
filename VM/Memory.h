

#define MEMSIZE 1677216  //内存大小16M
/*突然觉得......没有必要这样做，分页管理的话把一个char的数组分为很多个4096就可以了，逻辑页号就很简单了*/
struct MU  //内存单元，数据位是一个字节
{
        /* data */
        int available; //是否被分配出去
        int head; //是否是分配出去的头部
        int foot; //是否是分配出去的尾部
        int size; //如果是分配出去的头部或者尾部，指示块大小
        int next; //如果是空闲块，下一块空闲块头部的索引
        int index; //索引位
        char data; //数据位
};

struct Memory
{
        struct MU* data; //数据单元，初始化size个
        int size; //数据单元的个数，模拟16MB字节内存
};

void ReadMem(int addr,int size,char*buffer,void*Mem);
void WriteMen(int addr,int size,char*buffer,void*Mem);
