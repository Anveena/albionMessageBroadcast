//
// Created by tyran on 2024/8/26.
//

#include "flitter_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void convertIntToIp(unsigned ip, char *output) {
    sprintf(output, "%d.%d.%d.%d",
            (ip & 0xFF),
            (ip >> 8) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 24) & 0xFF);
}

int parseFilter(const char *input, char *strBuffer, unsigned int bufferSize) {
    char *str = strdup(input);
    char *tmp;
    const char *token = strtok_r(str, "-", &tmp);
    memset(strBuffer, '\0', bufferSize);
    int first = 1;
    while (token != NULL) {
        char *dotPos = strchr(token, '.');
        if (dotPos == NULL) {
            goto err1;
        }
        *dotPos = '\0';
        int ipInt = atoi(token);
        const char *port = dotPos + 1;
        char ipStr[16];
        convertIntToIp(ipInt, ipStr);
        if (first) {
            snprintf(strBuffer, bufferSize, "(src host %s and src port %s) or (dst host %s and dst port %s)", ipStr, port, ipStr, port);
        } else {
            char *tmpStr = strdup(strBuffer);
            snprintf(strBuffer, bufferSize, "%s or (src host  %s and src port %s) or (dst host %s and dst port %s)", tmpStr, ipStr, port, ipStr, port);
            free(tmpStr);
        }
        first = 0;
        token = strtok_r(NULL, "-", &tmp);
    }
    if (first != 0) {
        goto err1;
    }
    char *tmpStr = strdup(strBuffer);
    snprintf(strBuffer, bufferSize, "udp and (%s)", tmpStr);
    free(tmpStr);
    free(str);
    return 0;
    err1:
    free(str);
    snprintf(strBuffer, bufferSize, "输入的什么东西(%s)", input);
    return -1;
}
