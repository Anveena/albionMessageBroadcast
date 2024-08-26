//
// Created by tyran on 2024/8/26.
//

#include "args_parser.h"
#include <string.h>
#include <stdlib.h>

int parsePort(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-port") == 0) {
            if (i + 1 >= argc) {
                return -1;
            }
            return atoi(argv[i + 1]);
        }
    }
    return -1;
}

int parseProcessName(int argc, char *argv[], int *pNameIndex) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-pName") == 0) {
            if (i + 1 >= argc) {
                return -1;
            }
            *pNameIndex = i + 1;
            return 0;
        }
    }
    return -1;
}

int parseAddresses(int argc, char *argv[], int *filterIndex) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-addresses") == 0) {
            if (i + 1 >= argc) {
                return -1;
            }
            *filterIndex = i + 1;
            return 0;
        }
    }
    return -1;
}
