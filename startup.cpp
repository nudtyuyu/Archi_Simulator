#include <cstdio>

#define BUFSIZE 256
typedef unsigned int uint;

void load(char *textfile, char *datafile, uint taddr, uint daddr) {
    char *buf[BUFSIZE];
    FILE *fin = NULL;
    uint addr;
    int cnt;

    // load .text
    fin = fopen(textfile, "rb");
    addr = taddr;
    while (cnt = fread(buf, 1, BUFSIZE, fin)) {
        // MMU write(addr, cnt, buf)
        addr += cnt;
    }
    fclose(fin);

    // load .data
    fin = fopen(datafile, "rb");
    addr = daddr;
    while (cnt = fread(buf, 1, BUFSIZE, fin)) {
        // MMU write(addr, cnt, buf)
        addr += cnt;
    }
    fclose(fin);

    return;
}

void startup() {
    load("text", "data", 0x00010000, 0x10004000);
    // cpu.pc = 0x00010000
    // cpu.run()
}
