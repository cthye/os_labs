#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termio.h>
#include <unistd.h>

#include "tokenizer.h"

// macro silences compile warnings about unused function params
#define unused __attribute__((unused))

struct termios shell_tmodes;

typedef int cmd_fun_t(struct tokens *tokens);

#define MAX_HISTORY 10
char *history[MAX_HISTORY];
int cmd_cnt;

#define UP 38
#define DOWN 40

typedef struct fun_desc
{
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

int cmd_help(struct tokens *tokens);
int cmd_exit(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_history(struct tokens *tokens);

fun_desc_t cmd_tables[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_cd, "cd", "open directory"},
    {cmd_pwd, "pwd", "show current working directory"},
    {cmd_history, "history", "show recent 10 commands"},
};

int cmd_help(unused struct tokens *tokens)
{
    for (int i = 0; i < sizeof(cmd_tables) / sizeof(fun_desc_t); i++)
    {
        printf("%s - %s\n", cmd_tables[i].cmd, cmd_tables[i].doc);
    }
    return 1;
}

int cmd_exit(unused struct tokens *tokens)
{
    // store history cmd
    FILE *fd = fopen("./history.txt", "w+");
    if (fd != NULL)
    {
        for (int i = 0; i < MAX_HISTORY; i++)
        {
            if (history[i])
            {
                fprintf(fd, "%s", history[i]);
            }
        }
    }
    else
    {
        printf("file not found\n");
    }
    exit(0);
}

int cmd_cd(unused struct tokens *tokens)
{
    int ret = chdir(tokens_get_token(tokens, 1));
    if (!ret)
        return 1;
    else
    {
        printf("cann't find the directory\n");
        return -1;
    }
}

int cmd_pwd(unused struct tokens *tokens)
{
    char buf[4096];
    getcwd(buf, 4096);
    printf("%s\n", buf);
    return 1;
}

int cmd_history(unused struct tokens *tokens)
{
    for (int i = MAX_HISTORY - 1; i >= 0; i--)
    {
        if (history[i])
            printf("%s", history[i]);
    }
}

int lookup(char cmd[])
{
    for (int i = 0; i < sizeof(cmd_tables) / sizeof(fun_desc_t); i++)
    {
        if (cmd && strcmp(cmd_tables[i].cmd, cmd) == 0)
        {
            return i;
        }
    }
    return -1;
}

int shell_terminal;
int shell_is_interactive;
pid_t shell_pgid;

void init_shell()
{
    // connect shell to standard input
    // shell_terminal: fd of shell input
    shell_terminal = STDIN_FILENO;

    // check descriptor connected to the shell
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive)
    {
        // check whether the shell is the foreground process
        // if not, pause it until it becomes a foreground process
        // use SIGTTIN to pause
        // when the shell gets moved to the foreground , receive a SIGCONT
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
            kill(-shell_pgid, SIGTTIN);

        shell_pgid = getpid();
        tcsetpgrp(shell_terminal, shell_pgid);
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

// get enviroment params $PATH
int num_files = 0;
char *files[200];

void initPath()
{
    char *path = getenv("PATH");
    char file[4096];
    int n = 0;
    for (int i = 0; i < strlen(path); i++)
    {
        if (path[i] == ':')
        {
            char *copy = malloc(sizeof(char) * 4096);
            strcpy(copy, file);
            files[num_files++] = copy;
            memset(file, 0, n);
            n = 0;
        }
        else
        {
            file[n++] = path[i];
        }
    }
}

// find the src file of the command (token)
char *getFileFromPATH(char *token)
{
    for (int i = 0; i < num_files; i++)
    {
        char *filePath = malloc(sizeof(char) * 4096);
        strcpy(filePath, files[i]);
        strcat(filePath, "/");
        strcat(filePath, token);
        if (access(filePath, X_OK) != -1)
        {
            return filePath;
        }
        memset(filePath, 0, 4096);
    }
    printf("%s command not found\n", token);
    return 0;
}

void init_history()
{
    FILE *fd = fopen("./history.txt", "r");
    char line[256];
    if (fd != NULL)
    {
        int i = 0;
        while (fgets(line, sizeof(line), fd))
        {
            char *copy = malloc(sizeof(char) * 256);
            strcpy(copy, line);
            history[i++] = copy;
        }
        cmd_cnt = i;
    }
    else
    {
        printf("cannot open file");
    }
}

void add_cmd_to_history(char *cmd)
{
    if (cmd_cnt < MAX_HISTORY)
    {
        char *copy = malloc(sizeof(char) * 256);
        strcpy(copy, cmd);
        history[cmd_cnt++] = copy;
    }
    else
    {
        for (int i = 1; i < MAX_HISTORY; i++)
        {
            strcpy(history[i - 1], history[i]);
        }
        char *copy = malloc(sizeof(char) * 256);
        strcpy(copy, cmd);
        strcpy(history[MAX_HISTORY - 1], copy);
    }
}


int main(unused int argc, unused char *argv[])
{
    init_shell();
    initPath();
    init_history();

    static char line[4096];
    int line_num = 0;

    if (shell_is_interactive)
        fprintf(stdout, "\x1b[32m%d: ", line_num);

    char ch;
    char cnt = 0;
    char *line = malloc(sizeof(char) * 256);
    while (ch = getch())
    {
        if (ch == UP) {
            cmd_cnt --;
            

        }
        if (ch == '\n')
        {
            line[cnt] = '\n';

            add_cmd_to_history(line);
            struct tokens *tokens = tokenize(line);
            int fundex = lookup(tokens_get_token(tokens, 0));

            if (fundex >= 0)
            {
                // internal command
                cmd_tables[fundex].fun(tokens);
            }
            else
            {
                // externam command
                pid_t pid = fork();
                int status = 0;
                if (!pid)
                {
                    // in child
                    int tokenLen = tokens_get_length(tokens);
                    char **argv = malloc(sizeof(char *) * (tokenLen + 1));
                    int i;
                    for (i = 0; i < tokenLen; i++)
                    {
                        argv[i] = tokens_get_token(tokens, i);
                    }
                    argv[i] = NULL;
                    char *file = getFileFromPATH(tokens_get_token(tokens, 0));
                    execv(file, argv);
                    free(argv);
                    exit(0);
                }
                else
                {
                    // in parent
                    wait(&status);
                }
            }

            if (shell_is_interactive)
            {
                fprintf(stdout, "%d: ", ++line_num);
            }

            tokens_destroy(tokens);

            cnt = 0;
            memeset(line, 0, 256);
            continue;
        } 
    }
    return 0;
}
