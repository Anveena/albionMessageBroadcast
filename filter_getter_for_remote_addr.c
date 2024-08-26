#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include "args_parser.h"

// 获取特定进程的PID
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

// 格式化IP地址为整数
unsigned long formatIpToInt(const char *ipStr) {
    unsigned long a;
    unsigned long b;
    unsigned long c;
    unsigned long d;
    sscanf(ipStr, "%ld.%ld.%ld.%ld", &a, &b, &c, &d);
    return (d << 24) | (c << 16) | (b << 8) | a;
}

// 获取远程UDP地址
int getRemoteUDPAddr(const char *pName, char *strBuffer, unsigned int bufferSize) {
    DWORD pid = getPidByName(pName);
    if (pid == 0) {
        snprintf(strBuffer, bufferSize, "无法找到进程: %s", pName);
        return -1;
    }

    // 构建 netstat 命令
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "cmd.exe /c netstat -aon | findstr UDP");

    // 管道句柄和启动信息
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char buffer[4096];
    DWORD bytesRead;
    BOOL success;

    // 设置安全属性
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // 创建管道
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        snprintf(strBuffer, bufferSize, "CreatePipe失败");
        return -1;
    }

    // 设置子进程的启动信息
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = hWritePipe; // 子进程的标准错误输出重定向到管道写入端
    si.hStdOutput = hWritePipe; // 子进程的标准输出重定向到管道写入端
    si.dwFlags |= STARTF_USESTDHANDLES;

    // 创建子进程
    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        snprintf(strBuffer, bufferSize, "CreateProcess失败");
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        return -1;
    }

    // 关闭写入句柄以确保子进程终止后能够正确读取
    CloseHandle(hWritePipe);

    // 读取输出并解析
    strBuffer[0] = '\0'; // 清空输出缓冲区
    while (TRUE) {
        success = ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (!success || bytesRead == 0) break;
        buffer[bytesRead] = '\0'; // 确保缓冲区以 null 结尾
        for (char *line = strtok(buffer, "\n"); line != NULL; line = strtok(NULL, "\n")) {
            // 检查行是否包含 UDP 和非 * 的内容
            if (strstr(line, "UDP") == NULL || strstr(line, "*") != NULL) {
                continue;  // 不符合条件，跳过这行
            }
            // 提取PID
            DWORD linePid;
            sscanf(line + strlen(line) - 6, "%lu", &linePid); // 假设 PID 是输出行的最后一部分
            if (linePid != pid) {
                continue;  // 如果PID不匹配，跳过这行
            }

            // 提取远端地址
            char remoteAddr[256];
            sscanf(line, "%*s %*s %s", remoteAddr);

            // 分解远端地址和端口
            char *colonPos = strchr(remoteAddr, ':');
            if (colonPos != NULL) {
                *colonPos = '\0';
                unsigned long ipInt = formatIpToInt(remoteAddr);
                int portInt = atoi(colonPos + 1);
                char addrPort[64];
                snprintf(addrPort, sizeof(addrPort), "%lu.%d-", ipInt, portInt);
                // 追加到输出缓冲区
                if (strstr(strBuffer, addrPort) == NULL) {
                    strncat(strBuffer, addrPort, bufferSize - strlen(strBuffer) - 1);
                }
            }
        }
    }

    // 移除最后一个 '-'
    size_t len = strlen(strBuffer);
    if (len > 0 && strBuffer[len - 1] == '-') {
        strBuffer[len - 1] = '\0';
    }

    // 清理句柄
    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    char result[4096];
    int pNameIndex;
    if (parseProcessName(argc, argv, &pNameIndex) != 0) {
        fprintf(stderr, "filterGetterForRemoteAddr.exe无法正确启动,参数不合理,应该是-pName 进程名\n");
        return -1;
    }
    if (getRemoteUDPAddr(argv[pNameIndex], result, sizeof(result)) == 0) {
        fprintf(stdout, "%s", result);
    } else {
        fprintf(stderr, "filterGetter.exe无法正确启动,%s\n", result);
    }
    return 0;
}
