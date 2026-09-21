#include "kernel.h"
#include "System/desktop.h"

unsigned char inb(unsigned short port) {
    unsigned char r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}
void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
uint16_t inw(uint16_t port) {
    uint16_t r;
    __asm__ volatile("inw %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}
void io_wait(void) { outb(0x80, 0); }

void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}
uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

char* video_memory = (char*) 0xB8000;
int   cursor_pos = 0;
uint64_t boot_tsc = 0;

void update_cursor(int pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void scroll_if_needed(void) {
    if (cursor_pos < 80 * 25) return;
    for (int i = 0; i < 80 * 24; i++) {
        video_memory[i * 2]     = video_memory[(i + 80) * 2];
        video_memory[i * 2 + 1] = video_memory[(i + 80) * 2 + 1];
    }
    for (int i = 80 * 24; i < 80 * 25; i++) {
        video_memory[i * 2]     = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    cursor_pos = 80 * 24;
}

void clear_screen(void) {
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i * 2]     = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
    cursor_pos = 0;
    update_cursor(cursor_pos);
}

void print(char* str, char color) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_pos = ((cursor_pos / 80) + 1) * 80;
        } else {
            scroll_if_needed();
            video_memory[cursor_pos * 2]     = str[i];
            video_memory[cursor_pos * 2 + 1] = color;
            cursor_pos++;
        }
        i++;
    }
    scroll_if_needed();
    update_cursor(cursor_pos);
}

void print_int(int num, char color) {
    if (num == 0) { print("0", color); return; }
    if (num < 0) { print("-", color); num = -num; }
    char str[16];
    int i = 0;
    while (num > 0) { str[i++] = (num % 10) + '0'; num /= 10; }
    str[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char t = str[j]; str[j] = str[i-j-1]; str[i-j-1] = t;
    }
    print(str, color);
}

void print_uint64(uint64_t num, char color) {
    if (num == 0) { print("0", color); return; }
    char str[24];
    int i = 0;
    while (num > 0) { str[i++] = (num % 10) + '0'; num /= 10; }
    str[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char t = str[j]; str[j] = str[i-j-1]; str[i-j-1] = t;
    }
    print(str, color);
}

void draw_char(int x, int y, char c, char color) {
    int pos = y * 80 + x;
    video_memory[pos * 2]     = c;
    video_memory[pos * 2 + 1] = color;
}

int strcmp(char* s1, char* s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return (unsigned char)*s1 - (unsigned char)*s2;
}
int strlen(char* s) { int n = 0; while (s[n]) n++; return n; }
void strcpy(char* d, char* s) { while (*s) *d++ = *s++; *d = '\0'; }

multiboot_info_t* g_mbi = 0;
uint32_t total_ram_kb = 0;

void parse_multiboot(uint32_t magic, multiboot_info_t* mbi) {
    if (magic != 0x2BADB002) return;
    g_mbi = mbi;
    if (!(mbi->flags & (1 << 6))) return;
    uint32_t addr = mbi->mmap_addr;
    uint32_t end  = mbi->mmap_addr + mbi->mmap_length;
    while (addr < end) {
        uint32_t size = *(uint32_t*)addr;
        uint64_t len  = *(uint64_t*)(addr + 12);
        uint32_t type = *(uint32_t*)(addr + 20);
        if (type == 1) total_ram_kb += (uint32_t)(len / 1024);
        addr += size + 4;
    }
}

void keyboard_init(void) {
    outb(0x64, 0xAD); io_wait();
    outb(0x64, 0xA7); io_wait();
    while (inb(0x64) & 0x01) { inb(0x60); io_wait(); }
    outb(0x64, 0xAE); io_wait();
    while (inb(0x64) & 0x01) { inb(0x60); io_wait(); }
}

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    boot_tsc = rdtsc();   /* ← запоминаем момент загрузки */
    clear_screen();
    parse_multiboot(magic, mbi);
    keyboard_init();
    desktop_run();
}