// headers.h

#ifndef MY_SHELL_HEADERS_H
#define MY_SHELL_HEADERS_H
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


// Color Codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_USER    "\x1b[32m"  // green
#define COLOR_HOST    "\x1b[34m"  // blue
#define COLOR_DIR     "\x1b[33m"  // yellow
#define COLOR_BRACKET "\x1b[36m"  // cyan
#define COLOR_ERROR   "\x1b[31m"  // red


// ---- constants ----
#define MAX_LINE 4096
#define MAX_ARGS 128
#define HISTORY_SIZE 20

// ---- command struct ----
typedef struct {
    char *argv[MAX_ARGS];
    int argc;
    int is_background;
    char *input_file;
    char *output_file;
} Command;

// ---- globals ----
extern char shell_home[PATH_MAX];
extern char prev_dir[PATH_MAX];

extern char history_buf[HISTORY_SIZE][MAX_LINE];
extern int history_count;     // total stored (<= HISTORY_SIZE)
extern int history_start;     // index of oldest entry

extern pid_t current_fg_pid;

// ---- init / main loop ----
void init_shell(void);
void print_prompt(void);
int read_line(char *buffer, size_t size);
void handle_line(char *line);

// ---- parser ----
int parse_command(char *line, Command *cmd);

// ---- builtins ----
int is_builtin(Command *cmd);
int run_builtin(Command *cmd);
int run_builtin_with_redir(Command *cmd);   // ← add this

// ---- history ----
void load_history(void);
void save_history(void);
void add_history_entry(const char *line);
void show_history(void);

// ---- execute ----
void execute_command(Command *cmd);

// ---- signals ----
void setup_signal_handlers(void);
void sigint_handler(int signo);
void sigchld_handler(int signo);

#endif // MY_SHELL_HEADERS_H
