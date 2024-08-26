//
// Created by tyran on 2024/8/20.
//

#include "client.h"

bool writeAll(SOCKET clientSocket, const char *buffer, size_t size);

void freeClient(MY_CLIENT *client);

DWORD WINAPI mainLoop(LPVOID lpParam);

void addClient(SOCKET clientSocket, MY_BUFFER *buffer, MY_CLIENT **pClients,
               void (*onClientStatusChangedCallback)(const char *)) {
    MY_CLIENT *newClient = malloc(sizeof(MY_CLIENT));
    newClient->socket = clientSocket;
    newClient->working = 1;
    newClient->next = *pClients;
    newClient->localBuffer = malloc(65536 + 4);
    if (newClient->localBuffer == NULL) {
        free(newClient);
        return;
    }
    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    char *txt = malloc(1024);
    if (getpeername(clientSocket, (struct sockaddr *) &clientAddr, &addrLen) == 0) {
        snprintf(txt, 1024, "来自%lu.%lu.%lu.%lu:%d的新客户端已经接入",
                 (clientAddr.sin_addr.s_addr & 0xFF),
                 (clientAddr.sin_addr.s_addr >> 8 & 0xFF),
                 (clientAddr.sin_addr.s_addr >> 16 & 0xFF),
                 (clientAddr.sin_addr.s_addr >> 24 & 0xFF),
                 ntohs(clientAddr.sin_port));
    } else {
        snprintf(txt, 1024, "来自神秘地址(错误:%d)的新客户端已经接入", WSAGetLastError());
    }
    onClientStatusChangedCallback(txt);
    free(txt);
    newClient->semaphore = CreateSemaphore(NULL, 0, 65536, NULL);
    AcquireSRWLockShared(&buffer->rwlock);
    newClient->sn = buffer->sn;
    unsigned int length = buffer->currentBufferSize;
    memcpy(newClient->localBuffer, buffer->buffer, length);
    ReleaseSRWLockShared(&buffer->rwlock);
    if (!writeAll(clientSocket, newClient->localBuffer, length)) {
        free(newClient->localBuffer);
        free(newClient);
        return;
    }
    newClient->buffer = buffer;
    send(clientSocket, newClient->localBuffer, (int) length, 0);
    *pClients = newClient;
    newClient->thread = CreateThread(NULL, 0, mainLoop, newClient, 0, NULL);
}

void freeClients(MY_CLIENT **pClients) {
    MY_CLIENT *current = *pClients;
    MY_CLIENT *next = NULL;
    while (current != NULL) {
        next = current->next;
        if (current->semaphore != NULL) {
            CloseHandle(current->semaphore);
        }
        if (current->thread != NULL) {
            CloseHandle(current->thread);
        }
        if (current->socket != INVALID_SOCKET) {
            closesocket(current->socket);
        }
        if (current->localBuffer != NULL) {
            free(current->localBuffer);
        }
        free(current);
        current = next;
    }
    *pClients = NULL;
}

void broadcast(MY_CLIENT **clients) {
    MY_CLIENT *curr = *clients;
    MY_CLIENT *prev = NULL;
    while (curr) {
        ReleaseSemaphore(curr->semaphore, 1, NULL);
        if (InterlockedExchange(&curr->working, 1) == 0) {
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                *clients = curr->next;
            }
            MY_CLIENT *temp = curr->next;
            freeClient(curr);
            curr = temp;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

bool writeAll(SOCKET clientSocket, const char *buffer, size_t size) {
    if (size == 0) {
        return true;
    }
    size_t index = 0;
    while (true) {
        ssize_t writeLen = send(clientSocket, buffer + index, (int) (size - index), 0);
        if (writeLen <= 0) {
            return false;
        }
        index += writeLen;
        if (size == index) {
            return true;
        }
    }
}

void freeClient(MY_CLIENT *client) {
    WaitForSingleObject(client->thread, INFINITE);
    CloseHandle(client->thread);
    CloseHandle(client->semaphore);
    closesocket(client->socket);
    free(client->localBuffer);
    free(client);
}


DWORD WINAPI mainLoop(LPVOID lpParam) {
    MY_CLIENT *client = lpParam;
    SOCKET socket = client->socket;
    char *toWrite = client->localBuffer;
    unsigned int bufferSN;
    while (1) {
        if (WaitForSingleObject(client->semaphore, 10000) != WAIT_OBJECT_0) {
            continue;
        }
        client->buffer = client->buffer->next;
        client->sn++;
        AcquireSRWLockShared(&client->buffer->rwlock);
        bufferSN = client->buffer->sn;
        unsigned int length = client->buffer->currentBufferSize;
        memcpy(toWrite, client->buffer->buffer, length);
        ReleaseSRWLockShared(&client->buffer->rwlock);
        if (bufferSN != client->sn) {
            fprintf(stderr, "sn不匹配,可能接收端很慢: %u vs %u\n", bufferSN, client->sn);
            client->sn = bufferSN;
        }
        if (!writeAll(socket, toWrite, length)) {
            break;
        }
    }
    InterlockedExchange(&client->working, 0);
    return 0;
}
