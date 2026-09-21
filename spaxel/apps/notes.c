#include "apps.h"
#include "../kernel.h"
#include "../System/fs.h"
void app_notes(void) {
    char buf[128];
    int  len = 0;

    clear_screen();
    print("=== S-PAXEL NOTES v1.0 ===\n", 0x0E);
    print("Type text. F2 - save. ESC - exit.\n", 0x07);
    print("----------------------------------------\n", 0x07);

    buf[0] = '\0';

    unsigned char last = 0;
    unsigned char kbd_us[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
     '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
    };
    unsigned char kbd_shift[128] = {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
      '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
      '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' '
    };
    int shift = 0;

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);

        if (sc == 0xE0 || sc == 0xE1) continue;
        if (sc == 0x2A || sc == 0x36) { shift = 1; continue; }
        if (sc == 0xAA || sc == 0xB6) { shift = 0; continue; }
        if (sc >= 0x80) { last = 0; continue; }
        if (sc == last) continue;
        last = sc;

        /* ESC */
        if (sc == 0x01) return;

        /* F2 — сохранить */
        if (sc == 0x3C) {
            if (len == 0) {
                print("\n[Nothing to save]\n", 0x0C);
                continue;
            }
            int idx = fs_write_file("note.txt", buf);
            if (idx >= 0) {
                print("\n[Saved to /saved/note.txt]\n", 0x0A);
            } else {
                print("\n[Save failed]\n", 0x0C);
            }
            continue;
        }

        unsigned char c = shift ? kbd_shift[sc] : kbd_us[sc];
        if (c == 0) continue;

        if (c == '\n') {
            if (len < 127) { buf[len++] = '\n'; buf[len] = '\0'; }
            print("\n", 0x0F);
            continue;
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

        if (len < 127) {
            buf[len++] = c;
            buf[len] = '\0';
            char s[2] = { c, '\0' };
            print(s, 0x0F);
        }
    }
}