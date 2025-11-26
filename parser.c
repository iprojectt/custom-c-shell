#include "headers.h"

// Split operators < > & even if stuck to words
static void smart_split(char *line, char *tokens[], int *count) {
    int i = 0, t = 0;
    int len = strlen(line);

    while (i < len) {
        // skip spaces/tabs
        while (i < len && (line[i] == ' ' || line[i] == '\t'))
            i++;

        if (i >= len) break;

        // single-character operators
        if (line[i] == '<' || line[i] == '>' || line[i] == '&') {
            tokens[t] = malloc(2);
            tokens[t][0] = line[i];
            tokens[t][1] = '\0';
            t++;
            i++;
            continue;
        }

        // normal word
        int start = i;
        while (i < len && line[i] != ' ' && line[i] != '\t'
               && line[i] != '<' && line[i] != '>' && line[i] != '&')
            i++;

        int L = i - start;
        tokens[t] = malloc(L + 1);
        strncpy(tokens[t], line + start, L);
        tokens[t][L] = '\0';
        t++;
    }

    tokens[t] = NULL;
    *count = t;
}

int parse_command(char *line, Command *cmd) {
    cmd->argc = 0;
    cmd->is_background = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;

    char *tokens[MAX_ARGS];
    int token_count = 0;

    // NEW TOKENIZER
    smart_split(line, tokens, &token_count);

    if (token_count == 0)
        return -1;

    // process operators
    for (int i = 0; i < token_count; i++) {

        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 >= token_count) {
                fprintf(stderr, "syntax error: expected filename after '<'\n");
                return -1;
            }
            // copy filename so we can blank out tokens
            cmd->input_file = strdup(tokens[i + 1]);
            tokens[i][0] = '\0';
            tokens[i + 1][0] = '\0';
            i++;
        }
        else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 >= token_count) {
                fprintf(stderr, "syntax error: expected filename after '>'\n");
                return -1;
            }
            cmd->output_file = strdup(tokens[i + 1]);
            tokens[i][0] = '\0';
            tokens[i + 1][0] = '\0';
            i++;
        }
        else if (strcmp(tokens[i], "&") == 0) {
            cmd->is_background = 1;
            tokens[i][0] = '\0';
        }
    }

    // build argv (non-empty tokens only)
    for (int i = 0; i < token_count; i++) {
        if (tokens[i][0] != '\0')
            cmd->argv[cmd->argc++] = tokens[i];
    }

    cmd->argv[cmd->argc] = NULL;
    return 0;
}
