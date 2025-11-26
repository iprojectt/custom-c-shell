// history.c
#include "headers.h"

static void get_history_file_path(char *path, size_t size) {
    snprintf(path, size, "%s/%s", shell_home, "history.txt");
}

void load_history(void) {
    char path[PATH_MAX];
    get_history_file_path(path, sizeof(path));

    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (line[0] == '\0') continue;
        strncpy(history_buf[(history_start + history_count) % HISTORY_SIZE],
                line, MAX_LINE - 1);
        history_buf[(history_start + history_count) % HISTORY_SIZE][MAX_LINE - 1] = '\0';
        if (history_count < HISTORY_SIZE) {
            history_count++;
        } else {
            history_start = (history_start + 1) % HISTORY_SIZE;
        }
    }
    fclose(fp);
}

void save_history(void) {
    char path[PATH_MAX];
    get_history_file_path(path, sizeof(path));

    FILE *fp = fopen(path, "w");
    if (!fp) return;

    for (int i = 0; i < history_count; ++i) {
        int idx = (history_start + i) % HISTORY_SIZE;
        fprintf(fp, "%s\n", history_buf[idx]);
    }
    fclose(fp);
}

void add_history_entry(const char *line) {
    // ignore if same as last command
    if (history_count > 0) {
        int last_idx = (history_start + history_count - 1) % HISTORY_SIZE;
        if (strcmp(history_buf[last_idx], line) == 0) {
            return;
        }
    }

    int idx;
    if (history_count < HISTORY_SIZE) {
        idx = (history_start + history_count) % HISTORY_SIZE;
        history_count++;
    } else {
        // overwrite oldest
        idx = history_start;
        history_start = (history_start + 1) % HISTORY_SIZE;
    }

    strncpy(history_buf[idx], line, MAX_LINE - 1);
    history_buf[idx][MAX_LINE - 1] = '\0';
}

void show_history(void) {
    int to_show = history_count < 10 ? history_count : 10;
    int start = history_count - to_show;

    for (int i = 0; i < to_show; ++i) {
        int idx = (history_start + start + i) % HISTORY_SIZE;
        printf("%s\n", history_buf[idx]);
    }
}
