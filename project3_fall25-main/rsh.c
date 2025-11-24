#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12
#define MAXARGS 20

extern char **environ;

char *allowed[N] = {
    "cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff",
    "cd","exit","help"
};

int isAllowed(const char *cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void print_help() {
    printf("The allowed commands are:\n");
    printf("1: cp\n");
    printf("2: touch\n");
    printf("3: mkdir\n");
    printf("4: ls\n");
    printf("5: pwd\n");
    printf("6: cat\n");
    printf("7: grep\n");
    printf("8: chmod\n");
    printf("9: diff\n");
    printf("10: cd\n");
    printf("11: exit\n");
    printf("12: help\n");
}

int main() {

    char line[256];

    while (1) {

        fprintf(stderr, "rsh>");

        if (fgets(line, 256, stdin) == NULL) continue;

        if (strcmp(line, "\n") == 0) continue;

        line[strlen(line) - 1] = '\0';

        // ---- Tokenize input ----
        char *argv[MAXARGS + 2];
        int argc = 0;

        char *tok = strtok(line, " \t");
        while (tok != NULL && argc < MAXARGS + 1) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \t");
        }
        argv[argc] = NULL;

        if (argc == 0) continue;

        char *cmd = argv[0];

        // ---- Check allowed command ----
        if (!isAllowed(cmd)) {
            printf("NOT ALLOWED!\n");
            continue;
        }

        // ---- Built-in: exit ----
        if (strcmp(cmd, "exit") == 0) {
            return 0;
        }

        // ---- Built-in: help ----
        if (strcmp(cmd, "help") == 0) {
            print_help();
            continue;
        }

        // ---- Built-in: cd ----
        if (strcmp(cmd, "cd
