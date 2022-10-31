void itoa(char *buf, int num) {
    if (num < 0) {
        *(buf++) = '-';
        num = -num;
    }
    if (!num) {
        *(buf++) = '0';
        *(buf++) = '\0';
    } else {
        char *p = buf;
        while (num) {
            *(p++) = '0' + (char)(num % 10);
            num /= 10;
        }
        *(p--) = '\0';
        while (p > buf) {
            *p ^= *buf;
            *buf ^= *p;
            *p ^= *buf;
            ++buf, --p;
        }
    }
}
void write(char *buf) {
    __asm__("mv\ta0, %0\n\t" // buf
            "li\ta7, 4\n\t"
            "ecall\n" // sys_write
            :
            : "r"(buf)
            : "a0", "a7");
}
int vsprintf(char *buf, const char *fmt, char *args) {
    char *p;
    char tmp[256];

    for (p = buf; *fmt; fmt++) {
        if (*fmt != '%')
            *(p++) = *fmt;
        else {
            switch (*(++fmt)) {
                case 'd':
                    itoa(p, *(int *)args);
                    args += 4;
                    while (*p)
                        ++p;
                    break;
                case 's':
                    break;
                default:
                    break;
            }
        }
    }
    *(p++) = '\0';
}
int printf(const char *fmt, ...) {
    int i;
    char buf[256];
    char *args;
    __asm__("\taddi\t%0,s0,4" : "=r"(args));

    i = vsprintf(buf, fmt, args);
    write(buf);

    return i;
}

void exit() {
    __asm__("li\ta7,10\n\t"
            "ecall\n"
            : //
            : //
            : "a7");
}

void main() {
    int i, F1 = 1, F2 = 1, tmp;
    for (i = 3; i <= 10; i++) {
        F1 = F1 + F2;
        tmp = F1;
        F1 = F2;
        F2 = tmp;
    }
    printf("fib[10] = %d\n", F2);
    exit();
}