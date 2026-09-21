#include "fs.h"
#include "terminal.h"
#include "../kernel.h"
#include "../apps/apps.h"

FileNode fs[32] = {
    { "/",        1, -1, 0, 0, "" },
    { "apps",     1,  0, 0, 0, "" },
    { "games",    1,  1, 0, 0, "" },
    { "saved",    1,  0, 0, 0, "" },     /* ← новая папка */
    { "snake",    0,  2, 1, 0, "" },
    { "numbers",  0,  2, 1, 0, "" },
    { "parity",   0,  2, 1, 0, "" },
    { "calc",     0,  1, 1, 0, "" },
    { "notes",    0,  1, 1, 0, "" },
    { "info",     0,  1, 1, 0, "" },
    { "files",    0,  1, 1, 0, "" },
    { "editor",   0,  1, 1, 0, "" }
};
int fs_count = 12;   /* ← стало 11 */
int current_dir = 0;

void print_path(int dir) {
    if (dir == 0) { print("/", 0x0F); return; }
    print_path(fs[dir].parent_dir_id);
    print(fs[dir].name, 0x0F);
    print("/", 0x0F);
}

void cmd_ls(void) {
    print("Directory: ", 0x0E);
    print_path(current_dir);
    print("\n", 0x0F);
    int found = 0;
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == current_dir) {
            if (fs[i].is_dir) print("[DIR] ", 0x0B);
            else if (fs[i].is_app) print("[APP] ", 0x0E);
            else print("[TXT] ", 0x0F);
            print(fs[i].name, 0x0F);
            print("\n", 0x0F);
            found = 1;
        }
    }
    if (!found) print("  (empty)\n", 0x08);
}

void cmd_cd(char* name) {
    if (strcmp(name, "..") == 0) {
        if (current_dir != 0) current_dir = fs[current_dir].parent_dir_id;
        return;
    }
    if (strcmp(name, "/") == 0) { current_dir = 0; return; }
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == current_dir && fs[i].is_dir &&
            strcmp(fs[i].name, name) == 0) {
            current_dir = i;
            return;
        }
    }
    print("Directory not found!\n", 0x0C);
}

void cmd_cat(char* name) {
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == current_dir && !fs[i].is_dir &&
            strcmp(fs[i].name, name) == 0) {
            print(fs[i].content, 0x0F);
            print("\n", 0x0F);
            return;
        }
    }
    print("File not found!\n", 0x0C);
}

void cmd_mkdir(char* name) {
    if (fs_count >= 32) { print("FS full!\n", 0x0C); return; }
    FileNode* n = &fs[fs_count];
    int i = 0;
    while (name[i] && i < 15) { n->name[i] = name[i]; i++; }
    n->name[i] = '\0';
    n->is_dir = 1;
    n->parent_dir_id = current_dir;
    n->is_app = 0;
    n->deleted = 0;
    n->content[0] = '\0';
    fs_count++;
    print("Directory created.\n", 0x0A);
}

void cmd_rm(char* name) {
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == current_dir &&
            strcmp(fs[i].name, name) == 0) {

            print("Delete '", 0x0E);
            print(fs[i].name, 0x0B);
            print("'? (Y/N): ", 0x0E);

            unsigned char last = 0;
            while (1) {
                io_wait();
                if ((inb(0x64) & 0x01) == 0) continue;
                unsigned char sc = inb(0x60);
                if (sc >= 0x80) { last = 0; continue; }
                if (sc == last) continue;
                last = sc;

                if (sc == 0x15) {
                    print("Y\n", 0x0A);
                    fs[i].deleted = 1;
                    print("Deleted.\n", 0x0A);
                    return;
                }
                if (sc == 0x31) {
                    print("N\n", 0x08);
                    print("Cancelled.\n", 0x08);
                    return;
                }
            }
        }
    }
    print("Not found!\n", 0x0C);
}

void cmd_open(char* name) {
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == current_dir &&
            strcmp(fs[i].name, name) == 0) {
            if (fs[i].is_dir) { print("It's a directory.\n", 0x0C); return; }
            if (!fs[i].is_app) { print("Not an application.\n", 0x0C); return; }
            run_app_by_name(fs[i].name);
            clear_screen();
            print_prompt();
            return;
        }
    }
    print("Application not found!\n", 0x0C);
}

void cmd_apps(void) {
    print("=== INSTALLED APPS ===\n", 0x0E);
    for (int i = 1; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].is_app) {
            print("  ", 0x0F);
            print(fs[i].name, 0x0B);
            print("\n", 0x0F);
        }
    }
}
int fs_write_file(char* name, char* content) {
    int saved_id = -1;
    for (int i = 0; i < fs_count; i++) {
        if (fs[i].is_dir && strcmp(fs[i].name, "saved") == 0) {
            saved_id = i;
            break;
        }
    }
    if (saved_id < 0) {
        print("fs_write_file: no 'saved' dir!\n", 0x0C);
        return -1;
    }

    for (int i = 0; i < fs_count; i++) {
        if (fs[i].deleted) continue;
        if (fs[i].parent_dir_id == saved_id &&
            strcmp(fs[i].name, name) == 0) {
            int j = 0;
            while (content[j] && j < 127) { fs[i].content[j] = content[j]; j++; }
            fs[i].content[j] = '\0';
            return i;
        }
    }

    if (fs_count >= 32) {
        print("fs_write_file: FS full!\n", 0x0C);
        return -2;
    }

    FileNode* n = &fs[fs_count];
    int i = 0;
    while (name[i] && i < 15) { n->name[i] = name[i]; i++; }
    n->name[i] = '\0';
    n->is_dir = 0;
    n->parent_dir_id = saved_id;
    n->is_app = 0;
    n->deleted = 0;

    i = 0;
    while (content[i] && i < 127) { n->content[i] = content[i]; i++; }
    n->content[i] = '\0';

    fs_count++;
    return fs_count - 1;
}