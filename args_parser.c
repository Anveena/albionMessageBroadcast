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

int parseStringArg(int argc, char *argv[], const char *argName, int *idx) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], argName) == 0) {
            if (i + 1 >= argc) {
                return -1;
            }
            *idx = i + 1;
            return 0;
        }
    }
    return -1;
}

int parseProcessName(int argc, char *argv[], int *pNameIndex) {
    return parseStringArg(argc, argv, "-pName", pNameIndex);
}

int parseAddresses(int argc, char *argv[], int *filterIndex) {
    return parseStringArg(argc, argv, "-addresses", filterIndex);
}

int parseNetworkingDeviceKeywords(int argc, char *argv[], int *keywordsIndex) {
    return parseStringArg(argc, argv, "-deviceKeywords", keywordsIndex);
}