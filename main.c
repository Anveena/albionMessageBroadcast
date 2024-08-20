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

int main() {
    char *socketError;
    void *pRoom = initRoom(22550, 256, &socketError, &onSocketClientStatusChanged);
    HANDLE tcpListenThread = CreateThread(NULL, 0, broadcastMainloop, pRoom, 0, NULL);
    char *pcapErr = malloc(1024);
    pcap_t *handle = allocPcapHandler("loopback", pcapErr);
    if (handle == NULL) {
        return -1;
    }
    char *error;
    void *pMessagePacker = allocMessagePacker(&error);
    struct pcap_pkthdr *header;
    const u_char *packet;
    char *buffer;
    unsigned int bufferSize = 0;
    int rs = 0;
    while (pcap_next_ex(handle, &header, &packet) >= 0) {
        rs = transformPcapPacketToBuffer(pMessagePacker, packet, header->caplen, header->len,
                                         &buffer, &bufferSize);
        if (rs == 0) {
            broadcastData(pRoom, buffer, bufferSize);
        } else {
#ifdef PRINT_MY_LOG
            if (rs > 0) {
                fprintf(stdout, "%s\n", error);
            } else {
                fprintf(stderr, "%s\n", error);
            }
#endif
        }
    }
    WaitForSingleObject(tcpListenThread, INFINITE);
    CloseHandle(tcpListenThread);
    free(pcapErr);
    freePcapHandler(&handle);
    freeMessagePacker(&pMessagePacker);
    return 0;
}
