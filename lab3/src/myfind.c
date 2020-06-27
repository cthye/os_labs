#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <regex.h> 

#define MY_PATH_MAX 200

void help() {
    printf("myfind path -option [n|filename]\n");
    printf("options: -name -ctime -mtime\n");
    return;
}

int isNum(char* val) {
    for (int i = 0; i < strlen(val); i++) {
        if (!isdigit(val[i]))
            return 0;
    }
    return 1;
}

int matchString(char* curFile, char* filename) {
    int wildcard = 0;
    // int flag = 1;
    int fileIdx, curIdx;
    for(fileIdx = 0, curIdx = 0; fileIdx < strlen(filename) && curIdx < strlen(curFile);) {
        if (filename[fileIdx] == '?') {
            fileIdx++;
            curIdx++;
        } else if (filename[fileIdx] == curFile[curIdx]) {
            wildcard = 0;
            fileIdx++;
            curIdx++;
        } else if(filename[fileIdx] == '*') {
            //* todo: consider '*?'
            wildcard = 1;
            fileIdx++;
        } else {
            if (wildcard) {
                curIdx++;
            } else {
                return 0;
            }
        }
    }

    if (curIdx == strlen(curFile) && fileIdx < strlen(filename)) {
        // fix: myfind* cann't output myfind
        if(filename[fileIdx] == '*') return 1;
    }

    if(fileIdx != strlen(filename) || curIdx != strlen(curFile)) {
        if (wildcard) {
            for (fileIdx; fileIdx < strlen(filename); fileIdx++) {
                if(filename[fileIdx] != '*' || filename[fileIdx] != '?')
                    return 0;
            }
        } else {
            return 0;
        }
    } 
    
    return 1;
    
}

int compareTime(time_t fileTime, time_t curTime, int n) {
    double elaps = difftime(curTime, fileTime);
    if (elaps < 24*3600*(n+1)) {
        return 1;
    }
    return 0;
}

void find(int op, char* path, void*arg) {
    char* filename = NULL;
    long n = 0;
    if(op == 0) {
        filename = (char*)arg;
    }
    if(op == 1) {
        n = (long)arg;
    }
    if(op == 2) {
        n = (long)arg;
    }
    struct dirent *de;  
    DIR *dr = opendir(path); 
  
    if (dr == NULL) { 
        printf("Could not open current directory %s\n", path); 
        return; 
    } 

    while ((de = readdir(dr)) != NULL) {
        if (de->d_type == 4 && strcmp(de->d_name, ".") && strcmp(de->d_name,"..")) {
            char newPath[MY_PATH_MAX-1];
            memset(&newPath, 0, sizeof(newPath));
            strcat(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, de->d_name);
            find(op, newPath, arg);
        } else if(de->d_type == 8) {
            char curPath[MY_PATH_MAX-1];
            memset(&curPath, 0, sizeof(curPath));
            strcat(curPath, path);
            strcat(curPath, "/");
            strcat(curPath, de->d_name);
            struct stat stateStruc;
            if (lstat(curPath, &stateStruc) == -1) {
                perror("lstat");
            }
            time_t local;
            time(&local);

            if(op == 0) {
                if (!matchString(de->d_name, filename)) {
                    continue;
                }
            } else if(op == 1) {
                if(!compareTime(stateStruc.st_ctime, local, n)) {
                    continue;
                }
            } else if(op == 2) {
                if(!compareTime(stateStruc.st_mtime, local, n)) {
                    continue;
                }
            }
            
            printf("%s\n", curPath); 
        }
    }

    closedir(dr);     
}

int main(int argc, char* argv[]) {
    char* path;
    char* filename;
    int options;
    long n = 0;
    if (argc < 4) {
        help();
    } else {
        if(argv[2][0] != '-') help();
        else {
            path = argv[1];
            if(argv[2][1] == 'n') {
                filename = argv[3];
                options = 0;
                find(options, path, (void*)filename);
            } else {
                //* can't convert to int
                if(!isNum(argv[3])) help();
                else {
                    n = atoi(argv[3]);
                    if(argv[2][1] == 'c') {
                        options = 1;
                        find(options, path, (void*)n);
                    } else if(argv[2][1] == 'm') {
                        options = 2;
                        find(options, path, (void*)n);
                    } else help();
                }
            } 
        }
    }      
}