#include "apps.h"

void run_app_by_name(char* name) {
    if (strcmp(name, "snake") == 0) game_snake();
    else if (strcmp(name, "numbers") == 0) game_numbers();
    else if (strcmp(name, "parity") == 0) game_parity();
    else if (strcmp(name, "calc") == 0) app_calc();
    else if (strcmp(name, "notes") == 0) app_notes();
    else if (strcmp(name, "info") == 0) app_info();
    else if (strcmp(name, "files") == 0) app_files();
    else if (strcmp(name, "editor") == 0) app_editor();
    else print("Unknown app!\n", 0x0C);
} 