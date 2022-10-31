#include <cstdio>
#include "cpu.hpp"
#define BUFSIZE 64
typedef unsigned int uint;

void load(const char *file, uint addr) {
    char buf[BUFSIZE];
    FILE *fin = NULL;
    int cnt;

    // load .text
    fin = fopen(file, "rb");
    while (cnt = fread(buf, 1, 4, fin)) {
        char VA[32] = "";
        for (int i = 31, tmp = addr; i >= 0; i--) {
            VA[i] = (tmp & 1) + '0';
            tmp >>= 1;
        }
        // MMU write(addr, cnt, buf)
        MMU::WriteM(VA, buf, cnt);
        addr += cnt;
    }
    fclose(fin);

    return;
}

int main() {
    CPU::init();
    load("text", 0x00400000);
    load("data", 0x10010000);
    load("syscall", 0x00100000);
    // CPU::word = 1;
    // printf("0x%08x\n", (int)CPU::readmem(0x0010001c));
    // exit(0);
    CPU::pc = 0x00400334;
    // printf("0x%08x\n", &CPU::pc);
    CPU::cycle();
    return 0;
}
