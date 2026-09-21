#include "apps.h"

void app_info(void) {
    clear_screen();
    print("=== S-PAXEL SYSTEM INFO v1.0 ===\n", 0x0E);
    print("\n", 0x07);
    print("OS:      S-Paxel 1.0\n", 0x07);
    print("Kernel:  monolith, 32-bit protected mode\n", 0x07);
    print("Dev:     MrYarost\n", 0x07);
    print("Year:    2026\n", 0x07);
    print("\n[Press any key to return]\n", 0x0E);

    unsigned char last = 0;
    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);
        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;
        return;
    }
}