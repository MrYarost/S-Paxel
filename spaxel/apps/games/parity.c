#include "../apps.h"
#include "../../kernel.h"

extern uint64_t rdtsc(void);

static int my_rand(int max) {
    return (int)(rdtsc() % max);
}

static void wait_key(void) {
    unsigned char last = 0;
    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);
        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;
        if (sc == 0x01) return;
        return;
    }
}

void game_parity(void) {
    clear_screen();
    print("=========================================\n", 0x0E);
    print("     PARITY or NO PARITY X v1.0          \n", 0x0A);
    print("        Dev: MrYarost, 2026              \n", 0x07);
    print("=========================================\n", 0x0E);
    print("\n", 0x07);
    print("Hello, i Karl!\n", 0x0B);
    print("Choose:\n", 0x0B);
    print("  1 - NOT PARITY\n", 0x0C);
    print("  2 - PARITY\n", 0x0A);
    print("\nIf we match - WIN. If not - LOSE.\n", 0x0F);
    print("\nESC - exit\n", 0x07);
    print("\nPress any key to start...", 0x0E);
    wait_key();

    int lives = 5;

    while (1) {
        clear_screen();
        print("=========================================\n", 0x0E);
        print("         PARITY or NO PARITY X v1.0      \n", 0x0A);
        print("=========================================\n", 0x0E);
        print("\n", 0x07);
        print("Lives: ", 0x0F);
        for (int i = 0; i < lives; i++) print("*", 0x0C);
        print("\n\n", 0x07);

        print("Choose: 1=NOT PARITY, 2=PARITY\n", 0x0B);
        print("ESC - exit\n\n", 0x07);
        print("> ", 0x0A);

        unsigned char last = 0;
        int choice = 0;
        while (1) {
            io_wait();
            if ((inb(0x64) & 0x01) == 0) continue;
            unsigned char sc = inb(0x60);
            if (sc >= 0x80) { last = 0; continue; }
            if (sc == last) continue;
            last = sc;

            if (sc == 0x01) return;
            if (sc == 0x02) { choice = 1; break; }
            if (sc == 0x03) { choice = 2; break; }
        }

        int karl = 1 + my_rand(2);

        print("\n", 0x07);
        print("Karl chose: ", 0x0F);
        if (karl == 1) print("NOT PARITY\n", 0x0C);
        else           print("PARITY\n", 0x0A);

        if (choice == karl) {
            print("YOU WIN!\n", 0x0A);
        } else {
            print("YOU LOSE\n", 0x0C);
            lives--;
        }

        if (lives == 0) {
            print("\n=== GAME OVER ===\n", 0x0C);
            print("Press R to restart, ESC to exit\n", 0x0E);
            last = 0;
            while (1) {
                io_wait();
                if ((inb(0x64) & 0x01) == 0) continue;
                unsigned char sc = inb(0x60);
                if (sc >= 0x80) { last = 0; continue; }
                if (sc == last) continue;
                last = sc;
                if (sc == 0x01) return;
                if (sc == 0x13) { lives = 5; break; }
            }
            continue;
        }

        print("\nNext round...\n", 0x07);
        uint64_t t = rdtsc();
        while (rdtsc() - t < 1000000000ULL);
    }
}