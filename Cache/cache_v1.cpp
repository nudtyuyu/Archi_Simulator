#include <iostream>
#include "time.h"
using namespace std;

//TODO 如何保证结束时，cache里的脏数据回到memory

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory
class MM
{
private:
    char *data;

public:
    friend class CACHE;
    MM();
    ~MM();
};

MM::MM()
{
    data = (char *)malloc(sizeof(char) * 16384); // 16*1024
    for (int i = 0; i < 16384; i++)
    {
        data[i] = 0;
    }
}

MM::~MM()
{
    free(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cache项
class CACHE_ITEM
{
private:
    int tag;
    bool valid;
    bool dirty;
    char *data;

public:
    friend class CACHE;
    CACHE_ITEM();
    ~CACHE_ITEM();
};

CACHE_ITEM::CACHE_ITEM()
{
    tag = 0;
    valid = 0;
    dirty = 0;
    data = (char *)malloc(sizeof(char) * 16);
    for (int i = 0; i < 16; i++)
    {
        data[i] = 0;
    }
}

CACHE_ITEM::~CACHE_ITEM()
{
    free(data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Cache
class CACHE
{
private:
    CACHE_ITEM *items;

public:
    CACHE();
    ~CACHE();
    char read(int addr, MM *mem);
    void write(int addr, MM *mem, char data_writing);
};

CACHE::CACHE()
{
    items = new CACHE_ITEM[1024];
    // delete[] items;
    // for (int i = 0; i < 1024; i++)
    // {
    //     items[i] = CACHE_ITEM();
    // }
}

CACHE::~CACHE()
{
    delete[] items;
}

char CACHE::read(int addr, MM *mem)
{
    int addr_tag = (unsigned int)addr >> 4;
    int addr_offset = ((unsigned int)addr << 28) >> 28;
    // 如果查到有效项
    for (int i = 0; i < 1024; i++)
    {
        if (items[i].valid == 1)
        {
            if (items[i].tag == addr_tag)
            {
                return items[i].data[addr_offset];
            }
        }
    }
    // 未查到有效项
    // 随机替换
    srand(time(0));
    CACHE_ITEM &item_rand = items[rand() % 1024];

    // 写回
    if (item_rand.dirty)
    {
        for (int i = 0; i < 16; i++)
        {
            mem->data[item_rand.tag << 4 + i] = item_rand.data[i];
        }
    }
    // 读出来一块
    item_rand.tag = addr_tag;
    item_rand.valid = 1;
    item_rand.dirty = 0;
    for (int i = 0; i < 16; i++)
    {
        item_rand.data[i] = mem->data[addr_tag << 4 + i];
    }

    return item_rand.data[addr_offset];
}

void CACHE::write(int addr, MM *mem, char data_writing)
{
    int addr_tag = (unsigned int)addr >> 4;
    int addr_offset = ((unsigned int)addr << 28) >> 28;
    // 如果查到有效项
    for (int i = 0; i < 1024; i++)
    {
        if (items[i].valid == 1)
        {
            if (items[i].tag == addr_tag)
            {
                items[i].dirty = 1;
                items[i].data[addr_offset] = data_writing;
            }
        }
    }
    // 未查到有效项
    // 随机替换
    srand(time(0));

    CACHE_ITEM &item_rand = items[rand() % 1024];
    // 写回
    if (item_rand.dirty)
    {
        for (int i = 0; i < 16; i++)
        {
            mem->data[item_rand.tag << 4 + i] = item_rand.data[i];
        }
    }

    item_rand.tag = addr_tag;
    item_rand.valid = 1;
    item_rand.dirty = 1;
    for (int i = 0; i < 16; i++)
    {
        item_rand.data[i] = mem->data[addr_tag << 4 + i];
    }
    item_rand.data[addr_offset] = data_writing;
}

int main()
{
    MM mem = MM();
    CACHE cache = CACHE();

    cache.write(1, &mem, 11);
    cout << (int)cache.read(1, &mem) << endl;
    

    return 0;
}
