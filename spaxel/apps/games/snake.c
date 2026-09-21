#include "../apps.h"

uint64_t rdtsc(void);   /* из kernel.c */

#define SNAKE_MAX 200
#define FIELD_W 60
#define FIELD_H 18
#define FIELD_X 10
#define FIELD_Y 3

int snake_x[SNAKE_MAX];
int snake_y[SNAKE_MAX];
int snake_len;
int snake_dir;
int food_x, food_y;
int game_over;
int score;

static void snake_init(void) {
    snake_len = 3;
    snake_x[0] = FIELD_X + FIELD_W/2;  snake_y[0] = FIELD_Y + FIELD_H/2;
    snake_x[1] = snake_x[0] - 1;       snake_y[1] = snake_y[0];
    snake_x[2] = snake_x[0] - 2;       snake_y[2] = snake_y[0];
    snake_dir = 1;
    score = 0;
    game_over = 0;
    food_x = FIELD_X + 3 + (rdtsc() % (FIELD_W - 6));
    food_y = FIELD_Y + 2 + (rdtsc() % (FIELD_H - 4));
}

static void snake_draw_field(void) {
    for (int y = FIELD_Y - 1; y <= FIELD_Y + FIELD_H; y++)
        for (int x = FIELD_X - 1; x <= FIELD_X + FIELD_W; x++)
            draw_char(x, y, ' ', 0x00);

    for (int x = FIELD_X - 1; x <= FIELD_X + FIELD_W; x++) {
        draw_char(x, FIELD_Y - 1, '#', 0x08);
        draw_char(x, FIELD_Y + FIELD_H, '#', 0x08);
    }
    for (int y = FIELD_Y - 1; y <= FIELD_Y + FIELD_H; y++) {
        draw_char(FIELD_X - 1, y, '#', 0x08);
        draw_char(FIELD_X + FIELD_W, y, '#', 0x08);
    }
    draw_char(food_x, food_y, '*', 0x0C);
    for (int i = 0; i < snake_len; i++)
        draw_char(snake_x[i], snake_y[i], 'o', (i == 0) ? 0x0A : 0x02);

    print("Score: 000  WASD-move Q-quit 1/2-speed", 0x0F);
}

static void snake_step(void) {
    int nx = snake_x[0], ny = snake_y[0];
    if (snake_dir == 0) ny--;
    if (snake_dir == 1) nx++;
    if (snake_dir == 2) ny++;
    if (snake_dir == 3) nx--;

    if (nx < FIELD_X || nx >= FIELD_X + FIELD_W ||
        ny < FIELD_Y || ny >= FIELD_Y + FIELD_H) { game_over = 1; return; }
    for (int i = 0; i < snake_len; i++)
        if (snake_x[i] == nx && snake_y[i] == ny) { game_over = 1; return; }

    int ate = (nx == food_x && ny == food_y);
    if (!ate)
        draw_char(snake_x[snake_len-1], snake_y[snake_len-1], ' ', 0x00);
    else {
        if (snake_len < SNAKE_MAX) snake_len++;
        score += 10;
        food_x = FIELD_X + (rdtsc() % FIELD_W);
        food_y = FIELD_Y + (rdtsc() % FIELD_H);
    }

    for (int i = snake_len - 1; i > 0; i--) {
        snake_x[i] = snake_x[i-1];
        snake_y[i] = snake_y[i-1];
    }
    snake_x[0] = nx; snake_y[0] = ny;

    for (int i = 0; i < snake_len; i++)
        draw_char(snake_x[i], snake_y[i], 'o', (i == 0) ? 0x0A : 0x02);
    draw_char(food_x, food_y, '*', 0x0C);
}

void game_snake(void) {
    clear_screen();
    snake_init();
    snake_draw_field();

    unsigned char last = 0;
    uint64_t last_move = rdtsc();
    uint64_t speed = 400000000ULL;

    while (!game_over) {
        if (inb(0x64) & 0x01) {
            unsigned char sc = inb(0x60);
            if (sc >= 0x80) last = 0;
            else if (sc != last && sc != 0xE0 && sc != 0xE1) {
                last = sc;
                if (sc == 0x11)      snake_dir = 0;
                else if (sc == 0x1E) snake_dir = 3;
                else if (sc == 0x1F) snake_dir = 2;
                else if (sc == 0x20) snake_dir = 1;
                else if (sc == 0x10) game_over = 1;
                else if (sc == 0x02) { speed += 100000000ULL;
                    if (speed > 1500000000ULL) speed = 1500000000ULL; }
                else if (sc == 0x03) { if (speed > 100000000ULL) speed -= 100000000ULL; }
            }
        }
        if (rdtsc() - last_move > speed) {
            last_move = rdtsc();
            snake_step();
        }
    }

    print("\nGAME OVER! R-restart ESC-exit\n", 0x0C);

    last = 0;
    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char sc = inb(0x60);
            if (sc >= 0x80) { last = 0; continue; }
            if (sc == last) continue;
            last = sc;
            if (sc == 0x01) return;
            if (sc == 0x13) { game_snake(); return; }
        }
    }
}