//
// Created by tyran on 2024/8/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include "args_parser.h"

#pragma comment(lib, "iphlpapi.lib")

DWORD getPidByName(const char *processName) {
    DWORD pid = 0;
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if (Process32First(snapshot, &processEntry)) {
        do {
            if (strcmp(processName, processEntry.szExeFile) == 0) {
                pid = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    return pid;
}


int createFilterForPid(DWORD pid, const char *pName, char *strBuffer, unsigned int bufferSize) {
    PMIB_UDPTABLE_OWNER_PID udpTable;
    DWORD size = 0;
    DWORD ret;
    memset(strBuffer, 0, bufferSize);
    GetExtendedUdpTable(NULL, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    udpTable = (PMIB_UDPTABLE_OWNER_PID) malloc(size);
    if (udpTable == NULL) {
        snprintf(strBuffer, bufferSize, "内存都malloc失败了.");
        return -1;
    }
    ret = GetExtendedUdpTable(udpTable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    if (ret != NO_ERROR) {
        snprintf(strBuffer, bufferSize, "无法读取UDP监听表: %lu", ret);
        free(udpTable);
        return -1;
    }
    boolean got = FALSE;
    for (int i = 0; i < (int) udpTable->dwNumEntries; i++) {
        if (udpTable->table[i].dwOwningPid == pid) {
            snprintf(strBuffer, bufferSize, "%s-%lu.%hu",
                     strBuffer,
                     udpTable->table[i].dwLocalAddr,
                     ntohs((u_short) udpTable->table[i].dwLocalPort));
            got = TRUE;
        }
    }
    free(udpTable);
    if (got) {
        memcpy(strBuffer, strBuffer + 1, strlen(strBuffer) - 1);
        strBuffer[strlen(strBuffer) - 1] = 0;
        return 0;
    }
    snprintf(strBuffer, bufferSize, "%s 没有监听任何UDP", pName);
    return -1;
}


int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    int pNameIndex;
    if (parseProcessName(argc, argv, &pNameIndex) != 0) {
        fprintf(stderr, "参数不合理,应该是-pName 进程名");
        return -1;
    }

    unsigned int bufferSize = 2048;
    char *strBuffer = malloc(bufferSize);
    if (strBuffer == NULL) {
        fprintf(stderr, "内存都malloc失败了");
        return -1;
    }

    DWORD pid = getPidByName(argv[pNameIndex]);
    if (pid == 0) {
        fprintf(stderr, "没有名为 %s 的进程", argv[pNameIndex]);
        goto err1;
    }
    if (createFilterForPid(pid, argv[pNameIndex], strBuffer, bufferSize) != 0) {
        fprintf(stderr, "%s", strBuffer);
        goto err1;
    }
    fprintf(stderr, "%s", strBuffer);
    free(strBuffer);
    return 0;
    err1:
    free(strBuffer);
    return -1;
}