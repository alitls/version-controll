//
// Created by alireza on 5/29/25.
//
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

bool init();

int main(int argc, char *argv[]) {
    printf("Implementing part of git\n");

    printf("argc: %d\n", argc);
    if (argc < 2) {
        printf("Usage ./program.sh <command> [<args>]\n");
        return 1;
    }
    const char *command = argv[1];
    printf("arg: %s\n", command);

    if (strcmp(command, "init") == 0) {
        if (init()) {
            printf("Failed to create directories\n");
        }
        FILE *headFile = fopen(".git/HEAD", "w");
        if (headFile == NULL) {
            printf("Failed to create .git/HEAD file\n");
            return 1;
        }
        fprintf(headFile, "ref: refs/heads/main\n");
        fclose(headFile);

        printf("Init git directory\n");
    } else {
        printf("Unknown command %s\n", command);
        return 1;
    }

    return 0;
}

bool init() {
    return mkdir(".git", 0755) == -1 ||
           mkdir(".git/objects", 0755) == -1 ||
           mkdir(".git/refs", 0755) == -1;
}
