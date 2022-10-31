#include <iostream>
#include "time.h"
using namespace std;

// TODO 如何保证结束时，cache里的脏数据回到memory

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory
class MM
{
private:
    char *data;

public:
    friend class CACHE;
    friend class VICTIM_CACHE;
    MM();
    ~MM();
};

MM::MM()
{
    data = new char[16384]; // 16*1024
    for (int i = 0; i < 16384; i++)
    {
        data[i] = 0;
    }
}

MM::~MM()
{
    delete[] data;
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
    friend class VICTIM_CACHE;
    CACHE_ITEM();
    ~CACHE_ITEM();
};

CACHE_ITEM::CACHE_ITEM()
{
    tag = 0;
    valid = 0;
    dirty = 0;
    data = new char[16];
    for (int i = 0; i < 16; i++)
    {
        data[i] = 0;
    }
}

CACHE_ITEM::~CACHE_ITEM()
{
    delete[] data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Victim Cache
class VICTIM_CACHE
{
private:
    CACHE_ITEM *items;

public:
    friend class CACHE;
    VICTIM_CACHE();
    ~VICTIM_CACHE();
    char read(int addr);
    void replace(CACHE_ITEM *item, MM *mem);
};

VICTIM_CACHE::VICTIM_CACHE()
{
    items = new CACHE_ITEM[64];
}

VICTIM_CACHE::~VICTIM_CACHE()
{
    delete[] items;
}

char VICTIM_CACHE::read(int addr)
{
    int addr_tag = (unsigned int)addr >> 4;
    int addr_offset = ((unsigned int)addr << 28) >> 28;
    // 如果查到有效项
    for (int i = 0; i < 64; i++)
    {
        if (items[i].valid == 1)
        {
            if (items[i].tag == addr_tag)
            {
                return items[i].data[addr_offset];
            }
        }
    }

    return -1;
}

void VICTIM_CACHE::replace(CACHE_ITEM *item, MM *mem)
{
    // 如果查到相同项
    for (int i = 0; i < 64; i++)
    {
        if (items[i].valid == 1)
        {
            if (items[i].tag == item->tag)
            {
                items[i] = *item;
                return;
            }
        }
    }
    // 未查到有效项
    // 随机替换
    srand(time(0));

    CACHE_ITEM &item_rand = items[rand() % 64];
    if (item_rand.dirty) // 写回
    {
        for (int i = 0; i < 16; i++)
        {
            mem->data[(item_rand.tag << 4) + i] = item_rand.data[i];
        }
    }

    item_rand = *item;
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
    char read(int addr, MM *mem, VICTIM_CACHE vc);
    void write(int addr, MM *mem, char data_writing, VICTIM_CACHE vc);
};

CACHE::CACHE()
{
    items = new CACHE_ITEM[1024];
}

CACHE::~CACHE()
{
    delete[] items;
}

char CACHE::read(int addr, MM *mem, VICTIM_CACHE vc)
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

    // 未查到有效项，读VC
    if (vc.read(addr) != -1)
    {
        srand(time(0));
        CACHE_ITEM &item_rand = items[rand() % 1024];

        CACHE_ITEM item_temp = item_rand;
        for (int i = 0; i < 16; i++){
            if (vc.items[i].tag == addr_tag && vc.items[i].valid == 1){
                item_rand = vc.items[i];
                vc.items[i].tag = item_temp.tag;
                vc.items[i].valid = item_temp.valid;
                vc.items[i].dirty = item_temp.dirty;
                vc.items[i].data = item_temp.data;
                break;
            }
        }

        return item_rand.data[addr_offset];
    };

    // 随机替换
    srand(time(0));
    CACHE_ITEM &item_rand = items[rand() % 1024];
    vc.replace(&item_rand, mem); // 写入VC

    // 写回
    // if (item_rand.dirty)
    // {
    //     for (int i = 0; i < 16; i++)
    //     {
    //         mem->data[(item_rand.tag << 4) + i] = item_rand.data[i];
    //     }
    // }

    // 读出来一块
    item_rand.tag = addr_tag;
    item_rand.valid = 1;
    item_rand.dirty = 0;
    for (int i = 0; i < 16; i++)
    {
        item_rand.data[i] = mem->data[(addr_tag << 4) + i];
    }

    return item_rand.data[addr_offset];
}

void CACHE::write(int addr, MM *mem, char data_writing, VICTIM_CACHE vc)
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
                return;
            }
        }
    }
    // 未查到有效项
    if (vc.read(addr) != -1)
    {
        srand(time(0));
        CACHE_ITEM &item_rand = items[rand() % 1024];

        CACHE_ITEM item_temp = item_rand;
        for (int i = 0; i < 16; i++)
        {
            if (vc.items[i].tag == addr_tag && vc.items[i].valid == 1)
            {
                item_rand = vc.items[i];
                vc.items[i].tag = item_temp.tag;
                vc.items[i].valid = item_temp.valid;
                vc.items[i].dirty = item_temp.dirty;
                vc.items[i].data = item_temp.data;
                break;
            }
        }

        item_rand.data[addr_offset] = data_writing;
    };

    // 随机替换
    srand(time(0));

    CACHE_ITEM &item_rand = items[rand() % 1024];
    vc.replace(&item_rand, mem);

    // 写回
    // if (item_rand.dirty)
    // {
    //     for (int i = 0; i < 16; i++)
    //     {
    //         mem->data[(item_rand.tag << 4) + i] = item_rand.data[i];
    //     }
    // }

    item_rand.tag = addr_tag;
    item_rand.valid = 1;
    item_rand.dirty = 1;
    for (int i = 0; i < 16; i++)
    {
        item_rand.data[i] = mem->data[(addr_tag << 4) + i];
    }
    item_rand.data[addr_offset] = data_writing;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main
// int main()
// {
//     MM mem = MM();
//     CACHE cache = CACHE();
//     VICTIM_CACHE vc = VICTIM_CACHE();

//     cache.write(1, &mem, 11, vc);
//     cout << (int)cache.read(1, &mem, vc) << endl;

//     return 0;
// }
