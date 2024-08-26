//
// Created by tyran on 2024/8/19.
//

#include "message.h"
#include "macros.h"

typedef struct MyMessage {
    uint16_t dataLength;
    uint16_t isSnd;
    char *_;
} MY_MESSAGE;
typedef struct MessagePacker {
    MY_MESSAGE *snd;
    MY_MESSAGE *rcv;
    char *errorStr;
} MESSAGE_PACKER;

void *mallocMessagePacker(char *errorStr) {
    MESSAGE_PACKER *rs = malloc(sizeof(MESSAGE_PACKER));
    rs->errorStr = errorStr;
    rs->rcv = malloc(65536 + 4);
    rs->snd = malloc(65536 + 4);
    rs->rcv->isSnd = 0xff;
    rs->snd->isSnd = 0;
    return rs;
}

void freeMessagePacker(void **ppMessagePacker) {
    MESSAGE_PACKER *packer = *ppMessagePacker;
    free(packer->rcv);
    free(packer->snd);
    free(packer);
    *ppMessagePacker = NULL;
}

int transformPcapPacketToBuffer(void *pMessagePacker, boolean isRcv, const u_char *packet, unsigned int length,
                                char **buf, unsigned int *bufSize) {
    MESSAGE_PACKER *packer = pMessagePacker;
    int padding = MAC_LENGTH + 2;
    padding += 4 * (*(packet + padding) & 0b1111);
    unsigned int udp_length = ((unsigned int) *(packet + padding + 4)) << 8 | ((unsigned int) *(packet + padding + 5));
    if (udp_length + padding != length || udp_length == 0) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH,
                 "invalid packet:\n\tlength:%u,padding:%u,length_from_udp_header:%u",
                 length,
                 padding,
                 udp_length);
        return -1;
    }
    if (isRcv) {
        if (memcmp(packet + padding + 8, ((void *) packer->rcv) + 4, udp_length - 8) == 0) {
            snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH, "重复消息");
            return 1;
        }
        memcpy(((void *) packer->rcv) + 4, packet + padding + 8, udp_length - 8);
        packer->rcv->dataLength = (udp_length - 8) & 0xffff;
        *buf = (char *) packer->rcv;
        *bufSize = packer->rcv->dataLength + 4;
        return 0;
    }
    if (memcmp(packet + padding + 8, ((void *) packer->snd) + 4, udp_length - 8) == 0) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH, "重复消息");
        return 1;
    }
    memcpy(((void *) packer->snd) + 4, packet + padding + 8, udp_length - 8);
    packer->snd->dataLength = (udp_length - 8) & 0xffff;
    *buf = (char *) packer->snd;
    *bufSize = packer->snd->dataLength + 4;
    return 0;
}