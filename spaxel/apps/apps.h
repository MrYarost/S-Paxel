#ifndef APPS_H
#define APPS_H
#include "../System/fs.h"

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* Общие функции — определены в kernel.c */
void print(char* str, char color);
void print_int(int num, char color);
void clear_screen(void);
void draw_char(int x, int y, char c, char color);
int  strcmp(char* s1, char* s2);
int  strlen(char* s);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char data);
void io_wait(void);
int fs_write_file(char* name, char* content);
/* Прототипы приложений */
void app_calc(void);
void app_notes(void);
void app_info(void);
void app_files(void);
void app_editor(void);
void game_snake(void);
void game_pong(void);
void app_exit(void);
void game_parity(void);
void game_numbers(void);
/* Таблица приложений (в apps.c) */
void run_app_by_name(char* name);

#endif