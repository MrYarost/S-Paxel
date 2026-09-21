#include "apps.h"
#include "../kernel.h"
#include "../System/fs.h"

extern FileNode fs[32];
extern int      fs_count;
extern int      current_dir;

/* Временное состояние проводника (не трогает current_dir ФС) */
static int  sel_dir    = 0;
static int  selected   = 0;

/* Простой вывод файла */
static void show_file(int idx) {
    clear_screen();
    print("=== ", 0x0E);
    print(fs[idx].name, 0x0B);
    print(" ===\n", 0x0E);
    print("----------------------------------------\n", 0x07);
    print(fs[idx].content, 0x0F);
    print("\n----------------------------------------\n", 0x07);
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

/* Показать содержимое папки */
static void draw_files(void) {
    clear_screen();
    print("=== S-PAXEL FILES ===\n", 0x0E);
    print("Path: ", 0x0F);
    print_path(sel_dir);
    print("\n", 0x0F);
    print("----------------------------------------\n", 0x07);

    int found = 0;
    int count = 0;
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id != sel_dir) continue;

        if (count == selected) print("> ", 0x0A);
        else                   print("  ", 0x0F);

        if (fs[i].is_dir) print("[DIR] ", 0x0B);
        else              print("[TXT] ", 0x0F);

        print(fs[i].name, (count == selected) ? 0x0E : 0x0F);
        print("\n", 0x07);

        count++;
        found = 1;
    }
    if (!found) print("  (empty)\n", 0x08);

    print("\n----------------------------------------\n", 0x07);
    print("Up/Down - select  Enter - open\n", 0x07);
    print("Backspace - up  ESC - exit\n", 0x07);
}

/* Количество элементов в папке */
static int count_items(int dir) {
    int count = 0;
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == dir) count++;
    }
    return count;
}

/* Получить индекс i-го элемента в папке */
static int get_item(int dir, int n) {
    int count = 0;
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id != dir) continue;
        if (count == n) return i;
        count++;
    }
    return -1;
}

void app_files(void) {
    sel_dir  = 0;    /* старт с корня */
    selected = 0;

    draw_files();

    unsigned char last = 0;
    int extended = 0;

    while (1) {
        io_wait();
        if ((inb(0x64) & 0x01) == 0) continue;
        unsigned char sc = inb(0x60);

        if (sc == 0xE0) { extended = 1; continue; }
        if (sc == 0xE1) continue;

        if (sc >= 0x80) {
            unsigned char rel = sc - 0x80;
            if (rel == last) last = 0;
            extended = 0;
            continue;
        }

        if (sc == last && !extended) continue;
        last = sc;

        /* ESC */
        if (sc == 0x01) return;

        /* Стрелка вверх */
        if (extended && sc == 0x48) {
            extended = 0;
            if (selected > 0) selected--;
            draw_files();
            continue;
        }

        /* Стрелка вниз */
        if (extended && sc == 0x50) {
            extended = 0;
            int n = count_items(sel_dir);
            if (selected < n - 1) selected++;
            draw_files();
            continue;
        }

        /* W — вверх */
        if (sc == 0x11) {
            if (selected > 0) selected--;
            draw_files();
            continue;
        }

        /* S — вниз */
        if (sc == 0x1F) {
            int n = count_items(sel_dir);
            if (selected < n - 1) selected++;
            draw_files();
            continue;
        }

        /* Enter — открыть */
        if (sc == 0x1C) {
            int n = count_items(sel_dir);
            if (n == 0) continue;

            int idx = get_item(sel_dir, selected);
            if (idx < 0) continue;

            if (fs[idx].is_dir) {
                sel_dir = idx;
                selected = 0;
                draw_files();
            } else {
                show_file(idx);
                draw_files();
            }
            continue;
        }

        /* Backspace — наверх */
        if (sc == 0x0E) {
            if (sel_dir != 0) {
                sel_dir = fs[sel_dir].parent_dir_id;
                selected = 0;
                draw_files();
            }
            continue;
        }
    }
}