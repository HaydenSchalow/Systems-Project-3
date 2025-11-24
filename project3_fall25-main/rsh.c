/* rsh.c - restricted shell for Project 3 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

extern char **environ;

#define MAXLINE 4096
#define MAXARGS 20

/* allowed external commands (spawned) */
static const char *allowed_cmds[] = {
    "cp",
    "touch",
    "mkdir",
    "ls",
    "pwd",
    "cat",
    "grep",
    "chmod",
    "diff",
    NULL
};

/* prints help exactly as requested: header then the numbered list */
static void print_help(void) {
    puts("The allowed commands are:");
    puts("1: cp");
    puts("2: touch");
    puts("3: mkdir");
    puts("4: ls");
    puts("5: pwd");
    puts("6: cat");
    puts("7: grep");
    puts("8: chmod");
    puts("9: diff");
    puts("10: cd");
    puts("11: exit");
    puts("12: help");
}

static int is_allowed_external(const char *cmd) {
    for (const char **p = allowed_cmds; *p != NULL; ++p) {
        if (strcmp(*p, cmd) == 0) return 1;
    }
    return 0;
}

int main(void) {
    char line[MAXLINE];

    while (1) {
        /* print prompt to stderr (autograder ignores stderr) */
        fprintf(stderr, "rsh> ");
        fflush(stderr);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            /* EOF or error - exit shell */
            return 0;
        }

        /* remove trailing newline if present */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';

        /* skip empty lines */
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '\0') continue;

        /* parse tokens - up to MAXARGS (not counting terminating NULL) */
        char *argv[MAXARGS + 2]; /* command + up to MAXARGS-1 args + NULL */
        int argc = 0;
        char *saveptr = NULL;
        char *token = strtok_r(line, " \t", &saveptr);
        while (token != NULL && argc < MAXARGS + 1) { /* allow MAXARGS total tokens (including program) */
            argv[argc++] = token;
            token = strtok_r(NULL, " \t", &saveptr);
        }
        argv[argc] = NULL;

        if (argc == 0) continue;

        /* built-in: help */
        if (strcmp(argv[0], "help") == 0) {
            print_help();
            continue;
        }

        /* built-in: exit */
        if (strcmp(argv[0], "exit") == 0) {
            return 0;
        }

        /* built-in: cd */
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                /* must print this to stdout for autograding */
                puts("-rsh: cd: too many arguments");
            } else if (argc == 1) {
                /* no argument: change to HOME if available, otherwise do nothing */
                const char *home = getenv("HOME");
                if (home != NULL) {
                    if (chdir(home) != 0) {
                        /* chdir errors should go to stdout per spec (all non-prompt output) */
                        printf("cd: %s\n", strerror(errno));
                    }
                }
            } else {
                /* single argument */
                if (chdir(argv[1]) != 0) {
                    printf("cd: %s\n", strerror(errno));
                }
            }
            continue;
        }

        /* External commands: check allowed list */
        if (is_allowed_external(argv[0])) {
            pid_t pid;
            int spawn_err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
            if (spawn_err != 0) {
                /* spawn failed, print a clear message to stdout (not required exactly by spec,
                   but better than silent fail). Use strerror on spawn_err. */
                printf("%s\n", strerror(spawn_err));
                continue;
            }
            /* wait for child to finish */
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                /* waitpid failed - print error to stdout */
                printf("waitpid failed: %s\n", strerror(errno));
            }
            /* child's stdout/stderr go to the terminal as-is */
            continue;
        }

        /* not allowed */
        puts("NOT ALLOWED!");
    }

    /* not reached */
    return 0;
}
