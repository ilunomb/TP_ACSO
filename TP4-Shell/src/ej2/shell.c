#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_COMMANDS 256      // antes 200, ahora hasta 256 procesos en el pipeline
#define MAX_TOKENS    64
#define MAX_ARGS      (MAX_TOKENS + 1)
#define CMD_BUF_SIZE 4096     // antes 512, ahora 4096 para líneas largas

static int count_args(const char *s) {
    bool in_quote = false, in_tok = false;
    int cnt = 0;
    for (const char *p = s; *p; p++) {
        if (*p == '"') {
            if (!in_quote) {
                in_quote = true;
                if (!in_tok) { in_tok = true; cnt++; }
            } else {
                in_quote = false;
            }
        } else if (isspace((unsigned char)*p) && !in_quote) {
            if (in_tok) in_tok = false;
        } else {
            if (!in_tok) { in_tok = true; cnt++; }
        }
    }
    return cnt;
}

void split_args_preserving_quotes(char *input, char **args, int *arg_count) {
    char *p = input;
    *arg_count = 0;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) *p++ = '\0';
        if (!*p) break;
        if (*p == '"') {
            p++;
            args[(*arg_count)++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') { *p = '\0'; p++; }
        } else {
            args[(*arg_count)++] = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            if (*p) { *p = '\0'; p++; }
        }
        if (*arg_count >= MAX_TOKENS) break;
    }
    args[*arg_count] = NULL;
}

int main() {
    char command[CMD_BUF_SIZE];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }
        if (!fgets(command, sizeof(command), stdin))
            break;  // EOF

        // quitar '\n' y trim
        command[strcspn(command, "\n")] = '\0';
        char *start = command;
        while (*start && isspace((unsigned char)*start)) start++;
        char *end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end)) *end-- = '\0';
        memmove(command, start, strlen(start) + 1);

        // comillas sin cerrar
        int qc = 0;
        for (int i = 0; command[i]; i++) if (command[i] == '"') qc++;
        if (qc % 2 != 0) {
            fprintf(stderr, "Syntax error: comillas abiertas sin cerrar\n");
            continue;
        }

        if (strcmp(command, "exit") == 0) break;
        if (command[0] == '\0')       continue;

        size_t len = strlen(command);
        if (command[0] == '|' || command[len-1] == '|') {
            fprintf(stderr, "Syntax error: tubería al inicio o al final\n");
            continue;
        }
        if (strstr(command, "||")) {
            fprintf(stderr, "Syntax error: '||' no permitido\n");
            continue;
        }
        bool empty_between = false;
        for (char *p = command; (p = strchr(p, '|')); p++) {
            char *q = p + 1;
            while (*q && isspace((unsigned char)*q)) q++;
            if (*q == '|') { empty_between = true; break; }
        }
        if (empty_between) {
            fprintf(stderr, "Syntax error: comando vacío entre tuberías\n");
            continue;
        }

        // dividir por '|'
        command_count = 0;
        char *tok = strtok(command, "|");
        while (tok && command_count < MAX_COMMANDS) {
            commands[command_count++] = tok;
            tok = strtok(NULL, "|");
        }
        if (command_count == 0) continue;

        // chequeo de exceso de argumentos por etapa
        bool too_many = false;
        for (int i = 0; i < command_count; i++) {
            int actual = count_args(commands[i]);
            if (actual > MAX_TOKENS) {
                fprintf(stderr, "Syntax error: demasiados argumentos\n");
                too_many = true;
                break;
            }
        }
        if (too_many) continue;

        // crear pipes dinámicos según command_count
        int pipes[MAX_COMMANDS-1][2];
        for (int i = 0; i < command_count-1; i++) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(1);
            }
        }

        // fork + exec en cada etapa
        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }
            if (pid == 0) {
                if (i > 0)
                    dup2(pipes[i-1][0], STDIN_FILENO);
                if (i < command_count-1)
                    dup2(pipes[i][1], STDOUT_FILENO);
                for (int j = 0; j < command_count-1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                char *args[MAX_ARGS];
                int ac = 0;
                split_args_preserving_quotes(commands[i], args, &ac);
                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }
        }

        // padre cierra y espera a todos
        for (int i = 0; i < command_count-1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        for (int i = 0; i < command_count; i++)
            wait(NULL);
    }
    return 0;
}
