// execute.c
#include "headers.h"

void execute_command(Command *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // child

        // input redirection
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 input");
                close(fd);
                exit(1);
            }
            close(fd);
        }

        // output redirection
        if (cmd->output_file) {
            int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open output");
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 output");
                close(fd);
                exit(1);
            }
            close(fd);
        }

        // restore default handling of SIGINT in child
        signal(SIGINT, SIG_DFL);

        execvp(cmd->argv[0], cmd->argv);
        perror("execvp");
        exit(1);
    } else {
        // parent
        if (cmd->is_background) {
            printf("[bg] started pid %d\n", pid);
            // do not wait, no need to set current_fg_pid
        } else {
            current_fg_pid = pid;
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
            }
            current_fg_pid = -1;
        }
    }
}
