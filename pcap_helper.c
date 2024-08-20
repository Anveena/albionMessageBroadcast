//
// Created by tyran on 2024/8/19.
//

#include "pcap_helper.h"

pcap_t *allocPcapHandler(const char *deviceKeywords, char *errBuffer) {
    pcap_if_t *allDevices;
    pcap_if_t const *targetDevice = NULL;
    if (pcap_findalldevs(&allDevices, errBuffer) == -1) {
        fprintf(stderr, "can not find devices,error: %s\n", errBuffer);
        return NULL;
    }
    for (pcap_if_t *device = allDevices; device != NULL; device = device->next) {
        if (strstr(device->description, deviceKeywords) != NULL) {
            targetDevice = device;
            break;
        }
    }
    if (targetDevice == NULL) {
        fprintf(stderr, "can not find devices with keywords: %s\n", deviceKeywords);
        pcap_freealldevs(allDevices);
        return NULL;
    }
    pcap_t *handle = pcap_open_live(targetDevice->name, 65536, 1, 1000, errBuffer);
    if (handle == NULL) {
        pcap_freealldevs(allDevices);
        fprintf(stderr, "error opening device: %s\n", errBuffer);
        return NULL;
    }
    pcap_freealldevs(allDevices);
    char filter_exp[] = "udp port 1111";
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, filter_exp, 0, 0xffffffff) < 0) {
        pcap_freealldevs(allDevices);
        fprintf(stderr, "couldn't parse filter %s: %s\n", filter_exp, errBuffer);
        return NULL;
    }
    if (pcap_setfilter(handle, &fp) < 0) {
        pcap_freealldevs(allDevices);
        fprintf(stderr, "couldn't install filter %s: %s\n", filter_exp, errBuffer);
        return NULL;
    }
    return handle;
}

void freePcapHandler(pcap_t **ppPcapHandler) {
    pcap_close(*ppPcapHandler);
    *ppPcapHandler = NULL;
}