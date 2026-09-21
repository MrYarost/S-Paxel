#include "desktop.h"
#include "../kernel.h"
#include "../apps/apps.h"
#include "terminal.h"

/* ---------- иконки ---------- */
typedef struct {
    char* label;
    int   x;
    int   y;
    void (*run)(void);
} Icon;

static void launch_snake(void)    { game_snake(); }
static void launch_parity(void)   { game_parity(); }
static void launch_numbers(void)  { game_numbers(); }
static void launch_notes(void)    { app_notes();  }
static void launch_editor(void)   { app_editor();  }   /* пока редактор */
static void launch_files(void)    { app_files();  }   /* пока то же */
static void launch_terminal(void) { terminal_run(); }
static void launch_info(void)     { app_info();   }
static void launch_power(void)    { app_exit();   }

Icon icons[] = {
    /* Ряд 1 — игры */
    { "Snake",    20,  3, launch_snake    },
    { "Parity",   33,  3, launch_parity   },
    { "Numbers",  46,  3, launch_numbers  },
    /* Ряд 2 — работа с текстом */
    { "Notes",    20,  9, launch_notes    },
    { "Editor",   33,  9, launch_editor   },
    { "Files",    46,  9, launch_files    },
    /* Ряд 3 — система */
    { "Terminal", 20, 15, launch_terminal },
    { "Info",     33, 15, launch_info     },
    { "Power",    46, 15, launch_power    },
};
int icon_count = 9;
/* ---------- часы ---------- */
extern uint64_t rdtsc(void);
extern uint64_t boot_tsc;   /* сохраним в kernel_main */

static void draw_clock(int pos) {
    /* uptime в тиках (RDTSC) → в секунды (приблизительно) */
    uint64_t now = rdtsc();
    uint64_t diff = now - boot_tsc;

    /* Приблизительно: 3 ГГц → 3 млрд тиков/сек */
    /* Для точности надо калибровать, но пока ~3 GHz */
    uint64_t secs = diff / 3000000000ULL;
    uint32_t hh = (secs / 3600) % 24;
    uint32_t mm = (secs / 60) % 60;
    uint32_t ss = secs % 60;

    char buf[10];
    buf[0] = '0' + (hh / 10); buf[1] = '0' + (hh % 10);
    buf[2] = ':';
    buf[3] = '0' + (mm / 10); buf[4] = '0' + (mm % 10);
    buf[5] = ':';
    buf[6] = '0' + (ss / 10); buf[7] = '0' + (ss % 10);
    buf[8] = '\0';

    for (int i = 0; i < 8; i++) {
        video_memory[(pos + i) * 2]     = buf[i];
        video_memory[(pos + i) * 2 + 1] = 0x70;
    }
}
/* ---------- отрисовка иконки ---------- */
static void draw_icon(Icon* ic, int selected) {
    char bg_fill;
    char text_color;
    char border_color;

    if (selected) {
        /* Выделенная: чёрный фон, белый текст */
        bg_fill      = 0x00;   /* чёрный фон */
        text_color   = 0x0F;   /* белый текст */
        border_color = 0x0F;   /* белая рамка */
    } else {
        /* Обычная: белый фон, чёрный текст */
        bg_fill      = 0x00;   /* белый (светло-серый) фон */
        text_color   = 0x07;   /* чёрный текст */
        border_color = 0x00;   /* чёрная рамка */
    }

    /* Верхняя рамка (ширина 10) */
    draw_char(ic->x,      ic->y,     'v', border_color);
    for (int i = 1; i < 10; i++) draw_char(ic->x + i, ic->y, '-', border_color);
    draw_char(ic->x + 10, ic->y,     'v', border_color);

    /* Первая строка текста */
    draw_char(ic->x,      ic->y + 1, '|', border_color);
    for (int i = 1; i < 10; i++) draw_char(ic->x + i, ic->y + 1, ' ', bg_fill);
    draw_char(ic->x + 10, ic->y + 1, '|', border_color);

    /* Название (по центру, макс 8 символов) */
    int len = strlen(ic->label);
    if (len > 8) len = 8;
    int lx = ic->x + 5 - len / 2;
    for (int i = 0; i < len; i++) {
        draw_char(lx + i, ic->y + 1, ic->label[i], text_color);
    }

    /* Вторая строка */
    draw_char(ic->x,      ic->y + 2, '|', border_color);
    for (int i = 1; i < 10; i++) draw_char(ic->x + i, ic->y + 2, ' ', bg_fill);
    draw_char(ic->x + 10, ic->y + 2, '|', border_color);

    /* Нижняя рамка */
    draw_char(ic->x,      ic->y + 3, 'o', border_color);
    for (int i = 1; i < 10; i++) draw_char(ic->x + i, ic->y + 3, '-', border_color);
    draw_char(ic->x + 10, ic->y + 3, 'o', border_color);

    /* Подпись под иконкой, если имя длинное */
    if (strlen(ic->label) > 8) {
        int lx2 = ic->x + 5 - strlen(ic->label) / 2;
        if (lx2 < 0) lx2 = 0;
        for (int i = 0; ic->label[i]; i++) {
            draw_char(lx2 + i, ic->y + 4, ic->label[i],
                      selected ? 0x0F : 0x07);
        }
    }
}

/* ---------- отрисовка рабочего стола ---------- */
static void draw_desktop(int selected) {
    /* Тёмно-серый фон */
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i * 2]     = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    cursor_pos = 0;
    update_cursor(0);

    /* Заголовок (светлый текст на тёмно-сером) */
    print("                               S-PAXEL OS v1.0\n", 0x0B);
    print("                             ===================\n", 0x07);
    print("\n", 0x07);

    /* Иконки */
    for (int i = 0; i < icon_count; i++) {
        draw_icon(&icons[i], i == selected);
    }

    /* Панель задач — светло-серая */
    int pos = 24 * 80;
    for (int x = 0; x < 80; x++) {
        video_memory[(pos + x) * 2]     = ' ';
        video_memory[(pos + x) * 2 + 1] = 0x70;
    }
    char* bar = " WASD/Arrows-select  Enter-run  F1-F9-quick  T-terminal ";
    for (int i = 0; bar[i]; i++) {
        video_memory[(pos + i) * 2]     = bar[i];
        video_memory[(pos + i) * 2 + 1] = 0x70;
    }

    /* Часы справа */
    int clock_pos = 24 * 80 + 70;
    draw_clock(clock_pos);
}

/* ---------- главный цикл ---------- */
void desktop_run(void) {
    int selected = 0;
    draw_desktop(selected);

    unsigned char last = 0;
    int extended = 0;
    int cols = 3;

    uint64_t last_clock = rdtsc();

    while (1) {
        /* --- Обновление часов раз в секунду --- */
        uint64_t now = rdtsc();
        if (now - last_clock > 3000000000ULL) {   /* ~3 ГГц → 1 сек */
            draw_clock(24 * 80 + 70);
            last_clock = now;
        }

        /* --- Клавиатура --- */
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

        /* F1..F9 */
        if (sc >= 0x3B && sc <= 0x43) {
            int idx = sc - 0x3B;
            if (idx < icon_count) {
                icons[idx].run();
                draw_desktop(selected);
                last_clock = rdtsc();
            }
            extended = 0;
            continue;
        }

        /* T — терминал */
        if (sc == 0x14) {
            terminal_run();
            draw_desktop(selected);
            last_clock = rdtsc();
            extended = 0;
            continue;
        }

        /* Enter */
        if (sc == 0x1C) {
            icons[selected].run();
            draw_desktop(selected);
            last_clock = rdtsc();
            extended = 0;
            continue;
        }

        /* WASD */
        if (sc == 0x11) { if (selected >= cols) selected -= cols; draw_desktop(selected); last_clock = rdtsc(); continue; }
        if (sc == 0x1F) { if (selected + cols < icon_count) selected += cols; draw_desktop(selected); last_clock = rdtsc(); continue; }
        if (sc == 0x1E) { if (selected % cols > 0) selected--; draw_desktop(selected); last_clock = rdtsc(); continue; }
        if (sc == 0x20) { if (selected % cols < cols - 1 && selected + 1 < icon_count) selected++; draw_desktop(selected); last_clock = rdtsc(); continue; }

        /* Стрелки */
        if (extended) {
            extended = 0;
            if (sc == 0x4B) { if (selected % cols > 0) selected--; draw_desktop(selected); last_clock = rdtsc(); continue; }
            if (sc == 0x4D) { if (selected % cols < cols - 1 && selected + 1 < icon_count) selected++; draw_desktop(selected); last_clock = rdtsc(); continue; }
            if (sc == 0x48) { if (selected >= cols) selected -= cols; draw_desktop(selected); last_clock = rdtsc(); continue; }
            if (sc == 0x50) { if (selected + cols < icon_count) selected += cols; draw_desktop(selected); last_clock = rdtsc(); continue; }
        }
    }
}
