#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        //in child process
        // char cwd[100];
        // if(getcwd(cwd, sizeof(cwd)) != NULL)
        //     printf("Current working dir: %s\n", cwd);
        char * arg[] = {"./bin/an_ch2_1b", NULL};
        execv(arg[0], arg);
    } else {
        //in parent process
         int times = 50;
        while(times--) {
            time_t t = time(NULL);
            struct tm * local;
            local = localtime(&t);
            printf("Those output come from parent, %s \n", asctime(local));
            sleep(1);
        }
    }
}