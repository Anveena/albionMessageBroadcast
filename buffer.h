//
// Created by tyran on 2024/8/20.
//

#ifndef TRANSPORT_BUFFER_H
#define TRANSPORT_BUFFER_H

#include <windows.h>

typedef struct MyBuffer {
    unsigned int sn;
    unsigned int currentBufferSize;
    SRWLOCK rwlock;
    char *buffer;
    struct MyBuffer *next;
} MY_BUFFER;

MY_BUFFER *allocMyBuffers(unsigned int count);

void freeMyBuffers(MY_BUFFER *buffer);

#endif //TRANSPORT_BUFFER_H
