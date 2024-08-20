//
// Created by tyran on 2024/8/19.
//

#include "message.h"

#define ERROR_MESSAGE_LENGTH 1024

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

void *allocMessagePacker(char **errorStr) {
    MESSAGE_PACKER *rs = malloc(sizeof(MESSAGE_PACKER));
    rs->errorStr = malloc(ERROR_MESSAGE_LENGTH);
    rs->rcv = malloc(65536 + 4);
    rs->snd = malloc(65536 + 4);
    rs->rcv->isSnd = 0xff;
    rs->snd->isSnd = 0;
    *errorStr = rs->errorStr;
    return rs;
}

void freeMessagePacker(void **ppMessagePacker) {
    MESSAGE_PACKER *packer = *ppMessagePacker;
    free(packer->rcv);
    free(packer->snd);
    free(packer);
    *ppMessagePacker = NULL;
}

int
transformPcapPacketToBuffer(void *pMessagePacker, const u_char *packet, unsigned int capLength, unsigned int length,
                            char **buf, unsigned int *bufSize) {
    MESSAGE_PACKER *packer = pMessagePacker;
    if (packet == NULL) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH, "null ptr for packet");
        return -1;
    }
    if (capLength != length || length < 28 + IP_PACKET_PADDING) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH,
                 "invalid packet:\n\tpacketPtr:%p,length:%u,capLength:%u",
                 packet,
                 length,
                 capLength);
        return -1;
    }
    if ((*((const unsigned int *) packet)) != 2) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH,
                 "invalid packet:\n\tpacketPrefix4Byte:%u",
                 (*((const unsigned int *) packet)));
        return -1;
    }
    int padding = IP_PACKET_PADDING;
    if (*(packet + padding + 9) != 0x11) {
        snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH,
                 "invalid protocol type:%d", *(packet + padding + 9));
        return 1;
    }
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
    if (*(packet + padding) == 0x4 && *(packet + padding + 1) == 0x57) {
        if (memcmp(packet + padding + 8, ((void *) packer->snd) + 4, udp_length - 8) == 0) {
            snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH, "duplicate messages");
            return 1;
        }
        memcpy(((void *) packer->snd) + 4, packet + padding + 8, udp_length - 8);
        packer->snd->dataLength = (udp_length - 8) & 0xffff;
        *buf = (char *) packer->snd;
        *bufSize = packer->snd->dataLength + 4;
        return 0;
    }
    if (*(packet + padding + 2) == 0x4 && *(packet + padding + 3) == 0x57) {
        if (memcmp(packet + padding + 8, ((void *) packer->rcv) + 4, udp_length - 8) == 0) {
            snprintf(packer->errorStr, ERROR_MESSAGE_LENGTH, "duplicate messages");
            return 1;
        }
        memcpy(((void *) packer->rcv) + 4, packet + padding + 8, udp_length - 8);
        packer->rcv->dataLength = (udp_length - 8) & 0xffff;
        *buf = (char *) packer->rcv;
        *bufSize = packer->rcv->dataLength + 4;
        return 0;
    }
    return 1;
}