#include "terminal.h"
#include "../kernel.h"
#include "fs.h"
#include "../apps/apps.h"

unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};
unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
  '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' '
};
int shift_pressed = 0;

char cmd_buffer[80];
int  cmd_len = 0;

void print_prompt(void) {
    print("\nS-Paxel:", 0x0B);
    print_path(current_dir);
    print("> ", 0x0F);
}

void print_cpu_vendor(void) {
    uint32_t a, b, c, d;
    cpuid(0, &a, &b, &c, &d);
    char v[13];
    *(uint32_t*)&v[0] = b;
    *(uint32_t*)&v[4] = d;
    *(uint32_t*)&v[8] = c;
    v[12] = '\0';
    print(v, 0x0A);
}

void print_cpu_brand(void) {
    uint32_t a, b, c, d;
    cpuid(0x80000000, &a, &b, &c, &d);
    if (a < 0x80000004) { print("(brand N/A)", 0x08); return; }
    char brand[49];
    for (uint32_t i = 0; i < 3; i++) {
        cpuid(0x80000002 + i, &a, &b, &c, &d);
        *(uint32_t*)&brand[i*16 + 0]  = a;
        *(uint32_t*)&brand[i*16 + 4]  = b;
        *(uint32_t*)&brand[i*16 + 8]  = c;
        *(uint32_t*)&brand[i*16 + 12] = d;
    }
    brand[48] = '\0';
    char* p = brand;
    while (*p == ' ') p++;
    print(p, 0x0A);
}

void execute_command(void) {
    cmd_buffer[cmd_len] = '\0';
    print("\n", 0x0F);

    char cmd[20], arg[60];
    int i = 0, j = 0;
    while (cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0' && i < 19) {
        cmd[i] = cmd_buffer[i]; i++;
    }
    cmd[i] = '\0';
    if (cmd_buffer[i] == ' ') {
        i++;
        while (cmd_buffer[i] != '\0' && j < 59) arg[j++] = cmd_buffer[i++];
    }
    arg[j] = '\0';

    if (strcmp(cmd, "help") == 0) {
        print("Commands:\n", 0x0E);
        print("  help            - this help\n", 0x0F);
        print("  clear           - clear screen\n", 0x0F);
        print("  apps            - list installed apps\n", 0x0F);
        print("  open [name]     - run application\n", 0x0F);
        print("  ls              - list current dir\n", 0x0F);
        print("  cd [dir]        - change dir (.. or /)\n", 0x0F);
        print("  mkdir [name]    - create directory\n", 0x0F);
        print("  rm [name]       - delete (with confirm)\n", 0x0F);
        print("  cat [file]      - show file content\n", 0x0F);
        print("  hardware        - real CPU + RAM\n", 0x0F);
        print("  mem             - memory info\n", 0x0F);
        print("  snake           - shortcut for open snake\n", 0x0F);
        print("  esc             - back to desktop\n", 0x0F);
    }
    else if (strcmp(cmd, "clear") == 0) { clear_screen(); }
    else if (strcmp(cmd, "apps") == 0)  { cmd_apps(); }
    else if (strcmp(cmd, "open") == 0)  { cmd_open(arg); }
    else if (strcmp(cmd, "ls") == 0)    { cmd_ls(); }
    else if (strcmp(cmd, "cd") == 0)    { cmd_cd(arg); }
    else if (strcmp(cmd, "mkdir") == 0) { cmd_mkdir(arg); }
    else if (strcmp(cmd, "rm") == 0)    { cmd_rm(arg); }
    else if (strcmp(cmd, "cat") == 0)   { cmd_cat(arg); }
    else if (strcmp(cmd, "hardware") == 0) {
        print("=== REAL HARDWARE ===\n", 0x0E);
        print("CPU Vendor: ", 0x0F); print_cpu_vendor(); print("\n", 0x0F);
        print("CPU Brand:  ", 0x0F); print_cpu_brand();  print("\n", 0x0F);
        print("RAM Total:  ", 0x0F);
        print_int(total_ram_kb / 1024, 0x0A);
        print(" MB\n", 0x0F);
    }
    else if (strcmp(cmd, "mem") == 0) {
        print("=== MEMORY ===\n", 0x0B);
        print("Total RAM: ", 0x0F);
        print_int(total_ram_kb / 1024, 0x0A);
        print(" MB (", 0x0F);
        print_int(total_ram_kb, 0x0A);
        print(" KB)\n", 0x0F);

        /* Размер ядра */
        extern char _end;
        uint32_t kernel_size = (uint32_t)&_end - 0x100000;   /* 1 МБ */

        print("Kernel:    ", 0x0F);
        print_int(kernel_size / 1024, 0x0A);
        print(" KB (", 0x0F);
        print_int(kernel_size, 0x0A);
        print(" bytes)\n", 0x0F);

        /* Стек */
        print("Stack:     16 KB\n", 0x0F);

        /* ФС в RAM */
        uint32_t fs_size = 32 * (16 + 4 + 4 + 4 + 4 + 128);   /* 32 * sizeof(FileNode) */
        print("FS data:   ", 0x0F);
        print_int(fs_size / 1024, 0x0A);
        print(" KB\n", 0x0F);

        /* Итого */
        uint32_t used_kb = (kernel_size / 1024) + 16 + (fs_size / 1024);
        uint32_t free_kb = total_ram_kb > used_kb ? total_ram_kb - used_kb : 0;

        print("Used:      ", 0x0F);
        print_int(used_kb, 0x0A);
        print(" KB\n", 0x0F);

        print("Free:      ", 0x0F);
        print_int(free_kb / 1024, 0x0A);
        print(" MB\n", 0x0F);

        /* Процент */
        print("Usage:     ", 0x0F);
        if (total_ram_kb > 0) {
            /* Процент = used * 100 / total. Учитываем, что used ≈ 53 КБ, total ≈ 6 ГБ */
            uint32_t permille = (used_kb * 10000) / total_ram_kb;   /* *100, но в 0.01% */
            print_int(permille / 100, 0x0A);
            print(".", 0x0F);
            int frac = permille % 100;
            if (frac < 10) print("0", 0x0F);
            print_int(frac, 0x0A);
            print("%\n", 0x0F);
        } else {
            print("0%\n", 0x0F);
        }
    }
    else if (strcmp(cmd, "snake") == 0) {
        game_snake();
        clear_screen();
    }
    else if (cmd_len > 0) {
        print("Unknown command! Type 'help'.", 0x0C);
    }

    print_prompt();
    cmd_len = 0;
}

void handle_key(unsigned char scan_code) {
    if (scan_code == 0x2A || scan_code == 0x36) { shift_pressed = 1; return; }
    if (scan_code == 0xAA || scan_code == 0xB6) { shift_pressed = 0; return; }

    unsigned char ascii = shift_pressed ? kbd_us_shift[scan_code]
                                        : kbd_us[scan_code];
    if (ascii == 0) return;

    if (ascii == '\n') {
        execute_command();
    }
    else if (ascii == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            cursor_pos--;
            video_memory[cursor_pos * 2]     = ' ';
            video_memory[cursor_pos * 2 + 1] = 0x07;
            update_cursor(cursor_pos);
        }
    }
    else if (cmd_len < 79) {
        cmd_buffer[cmd_len++] = ascii;
        scroll_if_needed();
        video_memory[cursor_pos * 2]     = ascii;
        video_memory[cursor_pos * 2 + 1] = 0x0F;
        cursor_pos++;
        update_cursor(cursor_pos);
    }
}

void terminal_run(void) {
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i * 2]     = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    cursor_pos = 0;
    update_cursor(0);

    print("=========================================\n", 0x0F);
    print("       S-PAXEL TERMINAL v1.0             \n", 0x0B);
    print("     Type 'help' for commands            \n", 0x0F);
    print("       ESC - back to desktop             \n", 0x0F);
    print("=========================================\n", 0x0F);

    cmd_len = 0;
    print_prompt();

    unsigned char last = 0;

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);

        if (sc == 0xE0 || sc == 0xE1) continue;
        if (sc == 0x01) return;

        if (sc >= 0x80) {
            unsigned char rel = sc - 0x80;
            if (rel == 0x2A || rel == 0x36) handle_key(0xAA);
            if (rel == last) last = 0;
            continue;
        }

        if (sc == 0) continue;
        if (sc == 0x2A || sc == 0x36) { handle_key(sc); continue; }
        if (sc == last) continue;
        last = sc;
        handle_key(sc);
    }
}