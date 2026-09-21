#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_run(void);
void print_prompt(void);
void execute_command(void);
void handle_key(unsigned char scan_code);

#endif