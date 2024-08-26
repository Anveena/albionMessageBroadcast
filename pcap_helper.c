//
// Created by tyran on 2024/8/19.
//

#include "pcap_helper.h"
#include "macros.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

int getMacOfDevice(const char *deviceKeywords, unsigned char mac[6], char *errBuffer) {
    PIP_ADAPTER_INFO adapterInfo;
    PIP_ADAPTER_INFO tmpAdapterInfo;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    DWORD dwRetVal = 0;
    adapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    if (adapterInfo == NULL) {
        snprintf(errBuffer, ERROR_MESSAGE_LENGTH, "脑瘫Sonar:内存都malloc失败了");
        return -1;
    }
    if (GetAdaptersInfo(adapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(adapterInfo);
        adapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (adapterInfo == NULL) {
            snprintf(errBuffer, ERROR_MESSAGE_LENGTH, "脑瘫Sonar:内存都malloc失败了");
            return -1;
        }
    }
    if ((dwRetVal = GetAdaptersInfo(adapterInfo, &ulOutBufLen)) != NO_ERROR) {
        snprintf(errBuffer, ERROR_MESSAGE_LENGTH, "GetAdaptersInfo失败了:%lu", dwRetVal);
        free(adapterInfo);
        return -1;
    }
    tmpAdapterInfo = adapterInfo;
    boolean got = FALSE;
    while (tmpAdapterInfo) {
        if (strstr(tmpAdapterInfo->Description, deviceKeywords) != NULL) {
            memcpy(mac, tmpAdapterInfo->Address, 6);
            got = TRUE;
            break;
        }
        tmpAdapterInfo = tmpAdapterInfo->Next;
    }
    free(adapterInfo);
    if (!got) {
        return -1;
    }
    return 0;
}

pcap_t *allocPcapHandler(const char *deviceKeywords, const char *filter, char *errBuffer) {
    pcap_if_t *allDevices;
    pcap_if_t const *targetDevice = NULL;

    if (pcap_findalldevs(&allDevices, errBuffer) == -1) {
        fprintf(stderr, "调用pcap_findalldevs失败: %s\n", errBuffer);
        return NULL;
    }
    for (pcap_if_t *device = allDevices; device != NULL; device = device->next) {
        if (strstr(device->description, deviceKeywords) != NULL) {
            targetDevice = device;
            break;
        }
    }
    if (targetDevice == NULL) {
        fprintf(stderr, "没有和设备关键字匹配的设备: %s\n", deviceKeywords);
        pcap_freealldevs(allDevices);
        return NULL;
    }
    pcap_t *handle = pcap_open_live(targetDevice->name, 65536, 1, 1, errBuffer);
    if (handle == NULL) {
        pcap_freealldevs(allDevices);
        fprintf(stderr, "调用pcap_open_live失败: %s\n", errBuffer);
        return NULL;
    }
    pcap_freealldevs(allDevices);
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, filter, 0, 0xffffffff) < 0) {
        fprintf(stderr, "使用过滤器%s,调用pcap_compile失败: %s\n", filter, errBuffer);
        return NULL;
    }
    if (pcap_setfilter(handle, &fp) < 0) {
        fprintf(stderr, "使用过滤器%s,调用pcap_setfilter失败: %s\n", filter, errBuffer);
        return NULL;
    }
    return handle;
}

void freePcapHandler(pcap_t **ppPcapHandler) {
    pcap_close(*ppPcapHandler);
    *ppPcapHandler = NULL;
}