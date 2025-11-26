// signals.c
#include "headers.h"


void sigtstp_handler(int sig) {
    (void)sig;
    if (current_fg_pid > 0) {
        // Stop ONLY the foreground child
        kill(current_fg_pid, SIGTSTP);
    }

    // Do NOT stop the shell itself
    printf("\n");
    print_prompt();
    fflush(stdout);
}


void sigint_handler(int signo) {
    (void)signo;
    if (current_fg_pid > 0) {
        kill(current_fg_pid, SIGINT);
    } else {
        // no foreground process, ignore (shell continues)
        printf("\n");
        print_prompt();
        fflush(stdout);
    }
}

void sigchld_handler(int signo) {
    (void)signo;
    int status;
    pid_t pid;
    // reap all finished children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // optional: print message
        // printf("[bg] pid %d finished\n", pid);
    }
}

void setup_signal_handlers(void) {
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) < 0) {
        perror("sigaction SIGINT");
        exit(1);
    }

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) < 0) {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    // ******** ADD THIS BLOCK ********
    struct sigaction sa_tstp;
    memset(&sa_tstp, 0, sizeof(sa_tstp));
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) < 0) {
        perror("sigaction SIGTSTP");
        exit(1);
    }
}
