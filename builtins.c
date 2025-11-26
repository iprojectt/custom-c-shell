// builtins.c
#include "headers.h"

static int builtin_cd(Command *cmd) {
    char cwd[PATH_MAX];
    char target[PATH_MAX];

    // cd  → go to shell_home
    if (cmd->argc == 1) {
        if (!getcwd(cwd, sizeof(cwd))) cwd[0] = '\0';

        if (chdir(shell_home) != 0) {
            perror("cd");
            return -1;
        }

        if (cwd[0] != '\0')
            strcpy(prev_dir, cwd);
        return 0;
    }

    // too many args
    if (cmd->argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    const char *arg = cmd->argv[1];

    // cd -  → previous directory
    if (strcmp(arg, "-") == 0) {
        if (prev_dir[0] == '\0') {
            fprintf(stderr, "cd: no previous directory\n");
            return -1;
        }

        if (!getcwd(cwd, sizeof(cwd))) cwd[0] = '\0';

        if (chdir(prev_dir) != 0) {
            perror("cd");
            return -1;
        }

        printf("%s\n", prev_dir);  // print the directory we moved to

        if (cwd[0] != '\0')
            strcpy(prev_dir, cwd);

        return 0;
    }

    // ----- Tilde expansion -----
    if (arg[0] == '~') {
        if (arg[1] == '\0') {
            // "~"
            strncpy(target, shell_home, sizeof(target) - 1);
            target[sizeof(target) - 1] = '\0';
        } else if (arg[1] == '/') {
            // "~/something"
            snprintf(target, sizeof(target), "%s%s", shell_home, arg + 1);
        } else {
            // "~user" etc. – not required; just treat literally
            strncpy(target, arg, sizeof(target) - 1);
            target[sizeof(target) - 1] = '\0';
        }
    } else {
        // normal path, including ".."
        strncpy(target, arg, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
    }

    // ----- actually change directory -----
    if (!getcwd(cwd, sizeof(cwd))) cwd[0] = '\0';

    if (chdir(target) != 0) {
        perror("cd");
        return -1;
    }

    if (cwd[0] != '\0')
        strcpy(prev_dir, cwd);

    return 0;
}

static int builtin_pwd(Command *cmd) {
    (void)cmd;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("pwd");
        return -1;
    }
    printf("%s\n", cwd);
    return 0;
}

static int builtin_echo(Command *cmd) {
    for (int i = 1; i < cmd->argc; ++i) {
        if (i > 1) printf(" ");
        printf("%s", cmd->argv[i]);
    }
    printf("\n");
    return 0;
}

static int builtin_history(Command *cmd) {
    (void)cmd;
    show_history();
    return 0;
}

int is_builtin(Command *cmd) {
    const char *name = cmd->argv[0];
    return (strcmp(name, "cd") == 0 ||
            strcmp(name, "pwd") == 0 ||
            strcmp(name, "echo") == 0 ||
            strcmp(name, "history") == 0 ||
            strcmp(name, "exit") == 0);
}

int run_builtin(Command *cmd) {
    const char *name = cmd->argv[0];
    if (strcmp(name, "cd") == 0) {
        return builtin_cd(cmd);
    } else if (strcmp(name, "pwd") == 0) {
        return builtin_pwd(cmd);
    } else if (strcmp(name, "echo") == 0) {
        return builtin_echo(cmd);
    } else if (strcmp(name, "history") == 0) {
        return builtin_history(cmd);
    } else if (strcmp(name, "exit") == 0) {
        save_history();
        exit(0);
    }
    return -1;
}


int run_builtin_with_redir(Command *cmd) {
    int saved_stdin = -1, saved_stdout = -1;
    int in_fd = -1, out_fd = -1;

    // if input redirection: < file
    if (cmd->input_file) {
        in_fd = open(cmd->input_file, O_RDONLY);
        if (in_fd < 0) {
            perror("open input");
            return -1;
        }
        saved_stdin = dup(STDIN_FILENO);
        if (saved_stdin < 0) {
            perror("dup stdin");
            close(in_fd);
            return -1;
        }
        if (dup2(in_fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            close(in_fd);
            close(saved_stdin);
            return -1;
        }
        close(in_fd);
    }

    // if output redirection: > file
    if (cmd->output_file) {
        out_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd < 0) {
            perror("open output");
            if (saved_stdin >= 0) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            return -1;
        }
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout < 0) {
            perror("dup stdout");
            close(out_fd);
            if (saved_stdin >= 0) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            return -1;
        }
        if (dup2(out_fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            close(out_fd);
            close(saved_stdout);
            if (saved_stdin >= 0) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            return -1;
        }
        close(out_fd);
    }

    // run builtin normally (now stdin/stdout may be redirected)
    int ret = run_builtin(cmd);

    // restore stdin
    if (saved_stdin >= 0) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    // restore stdout
    if (saved_stdout >= 0) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }

    return ret;
}

