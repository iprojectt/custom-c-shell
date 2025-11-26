// main.c
#include "headers.h"

char shell_home[PATH_MAX];
char prev_dir[PATH_MAX];

char history_buf[HISTORY_SIZE][MAX_LINE];
int history_count = 0;
int history_start = 0;

pid_t current_fg_pid = -1;

void init_shell(void) {
    const char *home = getenv("HOME");

    if (home && *home) {
        // use OS user home as shell home
        strncpy(shell_home, home, sizeof(shell_home) - 1);
        shell_home[sizeof(shell_home) - 1] = '\0';
    } else {
        // fallback: current directory
        if (!getcwd(shell_home, sizeof(shell_home))) {
            perror("getcwd");
            exit(1);
        }
    }

    prev_dir[0] = '\0';

    load_history();
    setup_signal_handlers();
}


void print_prompt(void) {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd");
        strcpy(cwd, "?");
    }

    // username
    struct passwd *pw = getpwuid(getuid());
    const char *user = pw ? pw->pw_name : "user";

    // hostname
    char host[256];
    if (gethostname(host, sizeof(host)) != 0) {
        strcpy(host, "host");
    }

    // display directory with ~
    char display_dir[PATH_MAX];
    size_t home_len = strlen(shell_home);
    if (strncmp(cwd, shell_home, home_len) == 0) {
        if (cwd[home_len] == '\0') {
            strcpy(display_dir, "~");
        } else if (cwd[home_len] == '/') {
            snprintf(display_dir, sizeof(display_dir), "~%s", cwd + home_len);
        } else {
            // not actually inside home, fallback
            strcpy(display_dir, cwd);
        }
    } else {
        strcpy(display_dir, cwd);
    }



    // ***** COLORED PROMPT *****
    printf("%s<%s%s@%s%s:%s%s%s>%s ",
           COLOR_BRACKET,
           COLOR_USER, user,
           COLOR_HOST, host,
           COLOR_DIR, display_dir,
           COLOR_BRACKET,
           COLOR_RESET);

    fflush(stdout);



    // printf("<%s@%s:%s> ", user, host, display_dir);
    // fflush(stdout);
}

int read_line(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        // EOF (Ctrl+D)
        return 0;
    }
    // strip trailing newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return 1;
}

// process one single line: possibly multiple commands separated by ';'
void handle_line(char *line) {
    // trim leading spaces
    while (*line == ' ' || *line == '\t') line++;
    if (*line == '\0') return;  // empty

    // add to history if not duplicate
    add_history_entry(line);

    // split by ';'
    char *saveptr;
    char *cmd_str = strtok_r(line, ";", &saveptr);
    while (cmd_str) {
        // trim leading/trailing spaces of cmd_str
        while (*cmd_str == ' ' || *cmd_str == '\t') cmd_str++;
        char *end = cmd_str + strlen(cmd_str) - 1;
        while (end >= cmd_str && (*end == ' ' || *end == '\t')) {
            *end = '\0';
            end--;
        }

        if (*cmd_str != '\0') {
            Command cmd;
            if (parse_command(cmd_str, &cmd) == 0) {
                if (cmd.argc == 0) {
                    // nothing to execute
                } else if (is_builtin(&cmd)) {
                    // builtins execute in the shell process
                    if (cmd.input_file || cmd.output_file) {
                        run_builtin_with_redir(&cmd);
                    } else {
                        run_builtin(&cmd);
                    }
                } else {
                    // external command
                    execute_command(&cmd);
                }
            }
        }
        cmd_str = strtok_r(NULL, ";", &saveptr);
    }

    // save history periodically
    save_history();
}

int main(void) {
    init_shell();



    // ***** STARTUP BANNER *****
    printf("\n\x1b[36m----------------------------------------\n");
    printf("      Welcome to \x1b[32mOS_Mini_Project_C_Shell\x1b[36m\n");
    printf("----------------------------------------\x1b[0m\n\n");

    char line[MAX_LINE];
    while (1) {
        print_prompt();
        if (!read_line(line, sizeof(line))) {
            // Ctrl+D or EOF
            printf("\n");
            break;
        }

        // if line is only whitespace, skip without adding to history
        int only_space = 1;
        for (char *p = line; *p; ++p) {
            if (*p != ' ' && *p != '\t') {
                only_space = 0;
                break;
            }
        }
        if (only_space) continue;

        handle_line(line);
    }

    save_history();
    return 0;
}
