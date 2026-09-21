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
        return;
    }
}

void game_numbers(void) {
    clear_screen();
    print("=========================================\n", 0x0E);
    print("         NUMBERS X v1.0                  \n", 0x0A);
    print("      Dev: MrYarost, 2025                \n", 0x07);
    print("=========================================\n", 0x0E);
    print("\n", 0x07);
    print("I'll choose a range and a number.\n", 0x0B);
    print("You have 5 tries. I'll give hints.\n", 0x0B);
    print("\nESC - exit\n", 0x07);
    print("\nPress any key to start...", 0x0E);
    wait_key();

    int a1 = 1 + my_rand(150);
    int a2 = a1 + my_rand(150 - a1);
    if (a2 <= a1) a2 = a1 + 10;

    int target = a1 + my_rand(a2 - a1 + 1);

    unsigned char last = 0;

    while (1) {
        clear_screen();
        print("=========================================\n", 0x0E);
        print("         NUMBERS X v1.0                  \n", 0x0A);
        print("=========================================\n", 0x0E);
        print("\n", 0x07);
        print("Range: ", 0x0F);
        print_int(a1, 0x0A);
        print(" to ", 0x0F);
        print_int(a2, 0x0A);
        print("\n\n", 0x07);

        int tries = 5;
        int won = 0;

        while (tries > 0) {
            print("Tries left: ", 0x0F);
            print_int(tries, 0x0C);
            print("\n", 0x07);

            print("Your guess: ", 0x0B);
            int guess = 0;
            int digits = 0;
            last = 0;

            while (1) {
                io_wait();
                if ((inb(0x64) & 0x01) == 0) continue;
                unsigned char sc = inb(0x60);
                if (sc >= 0x80) { last = 0; continue; }
                if (sc == last) continue;
                last = sc;

                if (sc == 0x01) return;

                int d = -1;
                if (sc >= 0x02 && sc <= 0x0A) d = sc - 0x01;
                if (sc == 0x0B) d = 0;

                if (d >= 0 && digits < 5) {
                    guess = guess * 10 + d;
                    digits++;
                    print_int(d, 0x0F);
                }

                if (sc == 0x1C && digits > 0) {
                    print("\n", 0x07);
                    break;
                }

                if (sc == 0x0E && digits > 0) {
                    guess /= 10;
                    digits--;
                    cursor_pos--;
                    video_memory[cursor_pos * 2]     = ' ';
                    video_memory[cursor_pos * 2 + 1] = 0x07;
                    update_cursor(cursor_pos);
                }
            }

            if (guess == target) {
                print("\nCORRECT! The number was ", 0x0A);
                print_int(target, 0x0F);
                print("\n", 0x07);
                won = 1;
                break;
            } else if (guess < target) {
                print("MORE!\n", 0x0C);
                tries--;
            } else {
                print("LESS!\n", 0x0A);
                tries--;
            }
            print("\n", 0x07);
        }

        if (!won) {
            print("\n=== GAME OVER ===\n", 0x0C);
            print("The number was ", 0x0F);
            print_int(target, 0x0C);
            print("\n", 0x07);
        } else {
            print("\n=== YOU WIN! ===\n", 0x0A);
        }

        print("\nPress R to restart, ESC to exit\n", 0x0E);
        last = 0;
        while (1) {
            io_wait();
            if ((inb(0x64) & 0x01) == 0) continue;
            unsigned char sc = inb(0x60);
            if (sc >= 0x80) { last = 0; continue; }
            if (sc == last) continue;
            last = sc;
            if (sc == 0x01) return;
            if (sc == 0x13) {
                a1 = 1 + my_rand(150);
                a2 = a1 + my_rand(150 - a1);
                if (a2 <= a1) a2 = a1 + 10;
                target = a1 + my_rand(a2 - a1 + 1);
                break;
            }
        }
    }
}