#include <pcap.h>
#include <stdio.h>
#include <winsock2.h>
#include "message.h"
#include "pcap_helper.h"
#include "broadcasting_room.h"

#define PRINT_MY_LOG 0
//#undef PRINT_MY_LOG

void print_hex_to_stderr(const unsigned char *data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (i % 8 == 4) {
            fprintf(stderr, "\t");
        }
        fprintf(stderr, "%02X", data[i]);
        if (i < length - 1) {
            fprintf(stderr, " ");
        }
    }
    fprintf(stderr, "\n");
}

void onSocketClientStatusChanged(const char *desc) {
    fprintf(stderr, "%s\n", desc);
}

typedef struct PCapHelper {
    void *pMessagePacker;
    void *pRoom;
    char *buffer;
    char *pcapErr;
    unsigned int bufferSize;
} P_CAP_HELPER;

void packetHandler(u_char *user_data, const struct pcap_pkthdr *header, const u_char *packet) {
    P_CAP_HELPER *helper = (P_CAP_HELPER *) user_data;
    int rs = transformPcapPacketToBuffer(helper->pMessagePacker, packet, header->caplen, header->len,
                                         &helper->buffer, &helper->bufferSize);
    if (rs == 0) {
        broadcastData(helper->pRoom, helper->buffer, helper->bufferSize);
    } else {
#ifdef PRINT_MY_LOG
        if (rs > 0) {
            fprintf(stdout, "%s\n", helper->pcapErr);
        } else {
            fprintf(stderr, "%s\n", helper->pcapErr);
        }
#endif
    }
}

int parsePort(int argc, char *argv[]) {
    int port = 32999;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
                i++;
            } else {
                fprintf(stderr, "invalid port, use default port: %d", port);
                break;
            }
        }
    }
    return port;
}

int main(int argc, char *argv[]) {
    P_CAP_HELPER *helper = malloc(sizeof(P_CAP_HELPER));
    helper->buffer = NULL;
    helper->bufferSize = 0;
    char *error;
    helper->pMessagePacker = allocMessagePacker(&error);
    char *socketError;
    helper->pRoom = initRoom(parsePort(argc, argv), 1024, &socketError, &onSocketClientStatusChanged);

    HANDLE tcpListenThread = CreateThread(NULL, 0, broadcastMainloop, helper->pRoom, 0, NULL);

    helper->pcapErr = malloc(1024);
    pcap_t *handle = allocPcapHandler("loopback", helper->pcapErr);
    if (handle == NULL) {
        free(helper);
        return -1;
    }
    pcap_loop(handle, 0, packetHandler, (u_char *) helper);

    WaitForSingleObject(tcpListenThread, INFINITE);
    CloseHandle(tcpListenThread);
    free(helper->pcapErr);
    freePcapHandler(&handle);
    freeMessagePacker(&helper->pMessagePacker);
    return 0;
}
