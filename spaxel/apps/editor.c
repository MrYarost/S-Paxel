#include "apps.h"
#include "../kernel.h"
#include "../System/fs.h"

void app_editor(void) {
    clear_screen();
    print("=== S-PAXEL EDITOR v1.0 ===\n", 0x0E);
    print("Enter filename (max 15 chars), ESC - cancel:\n", 0x07);
    print("> ", 0x0A);

    /* --- Ввод имени файла --- */
    char filename[16];
    int  flen = 0;
    unsigned char last = 0;
    unsigned char kbd_us[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
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

        if (sc == 0x01) return;   /* ESC */

        unsigned char c = kbd_us[sc];
        if (c == 0) continue;

        if (c == '\n') {
            filename[flen] = '\0';
            print("\n", 0x07);
            break;
        }
        if (c == '\b') {
            if (flen > 0) {
                flen--;
                cursor_pos--;
                video_memory[cursor_pos * 2]     = ' ';
                video_memory[cursor_pos * 2 + 1] = 0x07;
                update_cursor(cursor_pos);
            }
            continue;
        }
        if (flen < 15) {
            filename[flen++] = c;
            char s[2] = { c, '\0' };
            print(s, 0x0F);
        }
    }

    if (flen == 0) return;

    /* --- Ввод текста --- */
    clear_screen();
    print("=== ", 0x0E);
    print(filename, 0x0B);
    print(" ===\n", 0x0E);
    print("Type text. F2 - save. ESC - exit.\n", 0x07);
    print("----------------------------------------\n", 0x07);

    char text[128];
    int  tlen = 0;
    text[0] = '\0';
    last = 0;

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);
        if (sc == 0xE0 || sc == 0xE1) continue;
        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;

        if (sc == 0x01) return;   /* ESC */

        /* F2 — сохранить */
        if (sc == 0x3C) {
            if (tlen == 0) {
                print("\n[Nothing to save]\n", 0x0C);
                continue;
            }
            int idx = fs_write_file(filename, text);
            if (idx >= 0) {
                print("\n[Saved to /saved/", 0x0A);
                print(filename, 0x0B);
                print("]\n", 0x0A);
            } else {
                print("\n[Save failed]\n", 0x0C);
            }
            continue;
        }

        unsigned char c = kbd_us[sc];
        if (c == 0) continue;

        if (c == '\n') {
            if (tlen < 127) { text[tlen++] = '\n'; text[tlen] = '\0'; }
            print("\n", 0x0F);
            continue;
        }

        if (c == '\b') {
            if (tlen > 0) {
                tlen--;
                cursor_pos--;
                video_memory[cursor_pos * 2]     = ' ';
                video_memory[cursor_pos * 2 + 1] = 0x07;
                update_cursor(cursor_pos);
            }
            continue;
        }

        if (tlen < 127) {
            text[tlen++] = c;
            text[tlen] = '\0';
            char s[2] = { c, '\0' };
            print(s, 0x0F);
        }
    }
}