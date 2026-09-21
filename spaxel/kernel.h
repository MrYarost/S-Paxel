#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

unsigned char inb(unsigned short port);
void          outb(unsigned short port, unsigned char data);
uint16_t      inw(uint16_t port);
void          io_wait(void);

extern uint64_t boot_tsc;
extern char* video_memory;
extern int   cursor_pos;

void update_cursor(int pos);
void scroll_if_needed(void);
void clear_screen(void);
void print(char* str, char color);
void print_int(int num, char color);
void print_uint64(uint64_t num, char color);
void draw_char(int x, int y, char c, char color);

int  strcmp(char* s1, char* s2);
int  strlen(char* s);
void strcpy(char* d, char* s);

void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d);
uint64_t rdtsc(void);

typedef struct {
    uint32_t flags, mem_lower, mem_upper, boot_device, cmdline;
    uint32_t mods_count, mods_addr, syms[4];
    uint32_t mmap_length, mmap_addr;
} multiboot_info_t;

extern multiboot_info_t* g_mbi;
extern uint32_t          total_ram_kb;

void parse_multiboot(uint32_t magic, multiboot_info_t* mbi);
void keyboard_init(void);
void run_app_by_name(char* name);

#endif