#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include "command.h"

typedef enum {
    CFG_NOFLAGS = 0,
    CFG_MUST_EXIST = 1 << 0,
    CFG_BUILTIN = 1 << 1
} ConfigFlags;

extern const char *config_file;
extern int config_line;

void list_builtin_configs(void);
const char *get_builtin_config(const char *const name);
void exec_config(const Command *cmds, const char *buf, size_t size);
int do_read_config(const Command *cmds, const char *filename, ConfigFlags f);
int read_config(const Command *cmds, const char *filename, ConfigFlags f);
void exec_builtin_rc(const char *rc);
void exec_reset_colors_rc(void);

#endif
