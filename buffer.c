//
// Created by tyran on 2024/8/20.
//

#include <assert.h>
#include "buffer.h"

void freeMyBuffers(MY_BUFFER *buffer) {
    for (MY_BUFFER *current = buffer; current == buffer;) {
        void *tmp = current->next;
        free(current->buffer);
        free(current);
        current = tmp;
    }
}

MY_BUFFER *allocMyBuffers(unsigned int count) {
    assert(count > 2);
    MY_BUFFER *first = malloc(sizeof(MY_BUFFER));
    first->buffer = malloc(65536 + 4);
    first->currentBufferSize = 0;
    InitializeSRWLock(&first->rwlock);
    MY_BUFFER *current = first;
    for (unsigned int i = 0; i < count - 1; ++i) {
        MY_BUFFER *buffer = malloc(sizeof(MY_BUFFER));
        buffer->buffer = malloc(65536 + 4);
        buffer->currentBufferSize = 0;
        InitializeSRWLock(&buffer->rwlock);
        current->next = buffer;
        current = buffer;
    }
    current->next = first;
    return first;
}