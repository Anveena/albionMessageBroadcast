#include <pcap.h>
#include <stdio.h>
#include <winsock2.h>
#include "message.h"
#include "pcap_helper.h"
#include "broadcasting_room.h"
#include "args_parser.h"
#include "macros.h"
#include "flitter_parser.h"


typedef struct PCapHelper {
    void *pMessagePacker;
    void *pRoom;
    char *buffer;
    char *pcapErr;
    char *packerErr;
    unsigned char mac[6];
    unsigned int bufferSize;
} P_CAP_HELPER;

void onSocketClientStatusChanged(const char *desc) {
    fprintf(stderr, "%s\n", desc);
}


void packetHandler(u_char *user_data, const struct pcap_pkthdr *header, const u_char *packet) {
    P_CAP_HELPER *helper = (P_CAP_HELPER *) user_data;
    if (packet == NULL || header->caplen != header->len || header->len < MAC_LENGTH + 4 + 20 + 8) {
        return;
    }
    int rs = transformPcapPacketToBuffer(helper->pMessagePacker,
                                         memcmp(packet, helper->mac, 6) == 0,
                                         packet,
                                         header->len,
                                         &helper->buffer, &helper->bufferSize);
    if (rs == 0) {
        broadcastData(helper->pRoom, helper->buffer, helper->bufferSize);
    } else {
#ifdef PRINT_MY_LOG
        if (rs > 0) {
            fprintf(stdout, "%s\n", helper->packerErr);
        } else {
            fprintf(stderr, "%s\n", helper->packerErr);
        }
#endif
    }
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    if (argv == NULL) {
        fprintf(stderr, "脑瘫Sonar:argv也能空??\n");
        return -1;
    }
    int filterIndex;
    if (parseAddresses(argc, argv, &filterIndex) != 0) {
        fprintf(stderr, "过滤器(-addresses)无法正确读取\n");
        return -1;
    }
    char *filter = malloc(DEFAULT_FILTER_BUFFER_SIZE);
    if (parseFilter(argv[filterIndex], filter, DEFAULT_FILTER_BUFFER_SIZE) != 0) {
        fprintf(stderr, "过滤器(-addresses)无法正确解析\n");
        free(filter);
        return -1;
    }
    fprintf(stderr, "通过过滤器\n\t%s\n开始抓包\n", filter);
    int port = parsePort(argc, argv);
    if (port < 0 || port > 65535) {
        port = DEFAULT_LISTEN_PORT;
        fprintf(stderr, "未有效的指定端口(-port),使用默认端口: %d\n", port);
    }

    P_CAP_HELPER *helper = malloc(sizeof(P_CAP_HELPER));
    if (helper == NULL) {
        fprintf(stderr, "脑瘫Sonar:内存都malloc失败了\n");
        free(filter);
        return -1;
    }
    helper->buffer = NULL;
    helper->bufferSize = 0;
    helper->packerErr = malloc(ERROR_MESSAGE_LENGTH);
    helper->pcapErr = malloc(ERROR_MESSAGE_LENGTH);
    if (helper->packerErr == NULL) {
        fprintf(stderr, "脑瘫Sonar:内存都malloc失败了\n");
        free(helper);
        return -1;
    }
    if (getMacOfDevice(DEFAULT_DEVICE_KEY_WORDS, helper->mac, helper->packerErr) != 0) {
        fprintf(stderr, "无法找到mac地址:%s\n", helper->packerErr);
        free(helper);
        return -1;
    }
    helper->pMessagePacker = mallocMessagePacker(helper->packerErr);

    char *socketError = malloc(ERROR_MESSAGE_LENGTH);
    helper->pRoom = mallocRoom(port, 1024, socketError, &onSocketClientStatusChanged);

    pcap_t *handle = allocPcapHandler(DEFAULT_DEVICE_KEY_WORDS, filter, helper->pcapErr);
    if (handle == NULL) {
        goto free;
    }
    HANDLE tcpListenThread = CreateThread(NULL, 0, broadcastMainloop, helper->pRoom, 0, NULL);

    pcap_loop(handle, 0, packetHandler, (u_char *) helper);

    WaitForSingleObject(tcpListenThread, INFINITE);

    free:
    free(filter);
    free(socketError);
    freeRoom(helper->pRoom);
    freeMessagePacker(&helper->pMessagePacker);
    free(helper->packerErr);
    free(helper);
    return 0;
}
