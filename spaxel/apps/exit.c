/* ============================================================
   apps/exit.c — Power menu
   ============================================================ */

#include "apps.h"

/* ---------- I/O для портов ---------- */
static void outw_power(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

/* ---------- Shutdown ---------- */
static void system_shutdown(void) {
    print("\nShutting down S-Paxel...\n", 0x0E);

    /* QEMU */
    outw_power(0x604,  0x2000);
    /* VirtualBox */
    outw_power(0x4004, 0x3400);
    /* Bochs */
    outw_power(0xB004, 0x2000);

    /* Если не сработало — hlt */
    print("(shutdown failed, halted)\n", 0x0C);
    while (1) __asm__ volatile("hlt");
}

/* ---------- Reboot ---------- */
static void system_reboot(void) {
    print("\nRebooting S-Paxel...\n", 0x0E);

    /* Ждём готовности i8042 */
    while (inb(0x64) & 0x02);

    /* Reset через i8042 */
    outb(0x64, 0xFE);

    /* Если не сработало — тройной fault */
    __asm__ volatile("cli");
    __asm__ volatile("lidt (%0)" : : "r"(0));
    __asm__ volatile("int $0x03");

    while (1) __asm__ volatile("hlt");
}

/* ---------- Power menu ---------- */
void app_exit(void) {
    clear_screen();
    print("\n", 0x0F);
    print("   POWER MENU:\n", 0x0E);
    print("\n", 0x0F);
    print("  [R]  Restart\n", 0x0A);
    print("  [S]  Shutdown\n", 0x0C);
    print("  [C]  Cancel\n", 0x0F);
    print("\n", 0x0F);
    print("   R | S | C\n", 0x0E);
    print("\n", 0x0F);
    unsigned char last = 0;

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);

        if (sc == 0xE0 || sc == 0xE1) continue;
        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;

        /* R = 0x13 */
        if (sc == 0x13) {
            system_reboot();
        }

        /* S = 0x1F */
        if (sc == 0x1F) {
            system_shutdown();
        }

        /* C = 0x2E, ESC = 0x01 */
        if (sc == 0x2E || sc == 0x01) {
            return;
        }
    }
}