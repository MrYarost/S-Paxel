#include "apps.h"
#include "../kernel.h"
#include "../System/fs.h"

/* Простой ввод строки с клавиатуры */
static int read_line(char* buf, int max) {
    int len = 0;
    unsigned char last = 0;

    unsigned char kbd_us[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+', '\b',
      '\t', '*', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
    };

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);
        if (sc == 0xE0 || sc == 0xE1) continue;

        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;

        if (sc == 0x01) return -1;   /* ESC */

        unsigned char c = kbd_us[sc];
        if (c == 0) continue;

        if (c == '\n') {
            buf[len] = '\0';
            print("\n", 0x07);
            return len;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                cursor_pos--;
                video_memory[cursor_pos * 2]     = ' ';
                video_memory[cursor_pos * 2 + 1] = 0x07;
                update_cursor(cursor_pos);
            }
            continue;
        }
        if (len < max - 1) {
            buf[len++] = c;
            char s[2] = { c, '\0' };
            print(s, 0x0F);
        }
    }
}

void app_calc(void) {
    clear_screen();
    print("=== S-PAXEL CALCULATOR v1.0 ===\n", 0x0E);
    print("Format: number op number   (op: +, -, *(Q), /)\n", 0x07);
    print("ESC - exit\n\n", 0x07);

    while (1) {
        print("> ", 0x0A);

        char line[40];
        int n = read_line(line, 40);
        if (n < 0) return;

        /* Парсим: первое число, оператор, второе число */
        int i = 0;
        int a = 0, b = 0;
        char op = 0;

        while (line[i] >= '0' && line[i] <= '9') {
            a = a * 10 + (line[i] - '0');
            i++;
        }
        if (line[i] == '+' || line[i] == '-' ||
            line[i] == '*' || line[i] == '/') {
            op = line[i]; i++;
        }
        while (line[i] >= '0' && line[i] <= '9') {
            b = b * 10 + (line[i] - '0');
            i++;
        }

        if (op == 0) { print("Bad input\n", 0x0C); continue; }

        int r = 0;
        if (op == '+') r = a + b;
        if (op == '-') r = a - b;
        if (op == '*') r = a * b;
        if (op == '/') {
            if (b == 0) { print("Division by zero!\n", 0x0C); continue; }
            r = a / b;
        }

        print("= ", 0x0A);
        print_int(r, 0x0F);
        print("\n", 0x07);
    }
}