#ifndef FS_H
#define FS_H

typedef struct {
    char name[16];
    int  is_dir;
    int  parent_dir_id;
    int  is_app;
    int  deleted;
    char content[128];
} FileNode;

extern FileNode fs[32];
extern int      fs_count;
extern int      current_dir;

void print_path(int dir);
void cmd_ls(void);
void cmd_cd(char* name);
void cmd_cat(char* name);
void cmd_mkdir(char* name);
void cmd_rm(char* name);
void cmd_open(char* name);
void cmd_apps(void);

/* NEW: записать файл в ФС */
int fs_write_file(char* name, char* content);

#endif