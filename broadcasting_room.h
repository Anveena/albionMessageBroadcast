//
// Created by tyran on 2024/8/20.
//

#ifndef TRANSPORT_BROADCASTING_ROOM_H
#define TRANSPORT_BROADCASTING_ROOM_H

#include <ws2tcpip.h>
#include <windows.h>

void *
initRoom(unsigned int listenAt, unsigned int bufferCount, char **errorStr,
         void (*onClientStatusChangedCallback)(const char *));

DWORD WINAPI broadcastMainloop(void *pRoom);

void broadcastData(void *pRoom, const char *data, unsigned int dataLen);

#endif //TRANSPORT_BROADCASTING_ROOM_H
