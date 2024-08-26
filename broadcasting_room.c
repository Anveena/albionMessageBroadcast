//
// Created by tyran on 2024/8/20.
//

#include "broadcasting_room.h"
#include "buffer.h"
#include "client.h"
#include "macros.h"
#include <windows.h>


typedef struct MyRoom {
    unsigned int sn;
    unsigned int listenAt;
    char *errorStr;
    MY_BUFFER *buffer;
    MY_CLIENT *clients;

    void (*onClientStatusChangedCallback)(const char *);
} MY_ROOM;

void *
mallocRoom(unsigned int listenAt, unsigned int bufferCount, char *errorStr,
           void (*onClientStatusChangedCallback)(const char *)) {
    MY_ROOM *room = malloc(sizeof(MY_ROOM));
    room->errorStr = errorStr;
    room->buffer = allocMyBuffers(bufferCount);
    room->sn = 0;
    room->clients = NULL;
    room->listenAt = listenAt;
    room->onClientStatusChangedCallback = onClientStatusChangedCallback;
    return room;
}

void freeRoom(void *pRoom) {
    MY_ROOM *room = pRoom;
    freeMyBuffers(room->buffer);
    freeClients(&room->clients);
    free(pRoom);
}

void broadcastData(void *pRoom, const char *data, unsigned int dataLen) {
    MY_ROOM *room = pRoom;
    room->sn++;
    room->buffer = room->buffer->next;
    AcquireSRWLockExclusive(&room->buffer->rwlock);
    memcpy(room->buffer->buffer, data, dataLen);
    room->buffer->currentBufferSize = dataLen;
    room->buffer->sn = room->sn;
    ReleaseSRWLockExclusive(&room->buffer->rwlock);
    broadcast(&room->clients);
}

DWORD WINAPI broadcastMainloop(void *pRoom) {
    MY_ROOM *room = pRoom;
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo hints;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        snprintf(room->errorStr, ERROR_MESSAGE_LENGTH,
                 "error when call func 'WSAStartup', return: %d", iResult);
        return 1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    char portStr[6];
    snprintf(portStr, sizeof(portStr), "%d", room->listenAt);
    iResult = getaddrinfo(NULL, portStr, &hints, &result);
    if (iResult != 0) {
        snprintf(room->errorStr, ERROR_MESSAGE_LENGTH,
                 "error when call func 'getaddrinfo', return: %d", iResult);
        WSACleanup();
        return 1;
    }

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        snprintf(room->errorStr, ERROR_MESSAGE_LENGTH,
                 "error when call func 'socket', return: %d", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        snprintf(room->errorStr, ERROR_MESSAGE_LENGTH,
                 "error when call func 'bind', return: %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        snprintf(room->errorStr, ERROR_MESSAGE_LENGTH,
                 "error when call func 'listen', return: %d", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    int count = 0;
    while (1) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }
        count++;
        if (count > 256) {
            break;
        }
        addClient(clientSocket, room->buffer, &room->clients, room->onClientStatusChangedCallback);
    }
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}