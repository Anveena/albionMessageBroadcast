//
// Created by tyran on 2024/8/20.
//

#ifndef TRANSPORT_CLIENT_H
#define TRANSPORT_CLIENT_H

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include "buffer.h"

typedef struct MyClient {
    HANDLE semaphore;
    HANDLE thread;
    SOCKET socket;
    LONG working;
    MY_BUFFER *buffer;
    char *localBuffer;
    unsigned int sn;
    struct MyClient *next;
} MY_CLIENT;

void addClient(SOCKET clientSocket, MY_BUFFER *buffer, MY_CLIENT **pClients,
               void (*onClientStatusChangedCallback)(const char *));

void freeClients(MY_CLIENT **pClients);

void broadcast(MY_CLIENT **clients);

#endif //TRANSPORT_CLIENT_H
