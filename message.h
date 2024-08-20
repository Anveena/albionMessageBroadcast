//
// Created by tyran on 2024/8/19.
//

#ifndef TRANSPORT_MESSAGE_H
#define TRANSPORT_MESSAGE_H

#include <pcap.h>
#include <stdint.h>

#define IP_PACKET_PADDING 4

void *allocMessagePacker(char **errorStr);

void freeMessagePacker(void **ppMessagePacker);

int
transformPcapPacketToBuffer(void *pMessagePacker, const u_char *packet, unsigned int capLength, unsigned int length,
                            char **buf, unsigned int *bufSize);

#endif //TRANSPORT_MESSAGE_H