#include "mmu.hpp"
namespace SHOW {
    void init() {
        int addr = 0x00010000;
        while (addr < 0x00011000) {
            char VA[33] = "";
            for (int i = 31, tmp = addr; i >= 0; i--) {
                VA[i] = tmp & 1 ? '1' : '0';
                tmp >>= 1;
            }
            MMU::WriteM(VA, "\0", 1);
            ++addr;
        }
    }
    void show() {
        int addr = 0x00010000;
        while (1) {
            char c, VA[33] = "";
            for (int i = 31, tmp = addr; i >= 0; i--) {
                VA[i] = tmp & 1 ? '1' : '0';
                tmp >>= 1;
            }
            MMU::ReadM(VA, &c, 1);
            if (!c)break;
            printf("%c", c);
            for (int i = 31, tmp = addr; i >= 0; i--) {
                VA[i] = tmp & 1 ? '1' : '0';
                tmp >>= 1;
            }
            MMU::WriteM(VA, "\0", 1);
            ++addr;
        }
    }
} // namespace SHOW