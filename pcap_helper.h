//
// Created by tyran on 2024/8/19.
//

#ifndef TRANSPORT_PCAP_HELPER_H
#define TRANSPORT_PCAP_HELPER_H

#include <pcap.h>

int getMacOfDevice(const char *deviceKeywords, unsigned char mac[6], char *errBuffer);

pcap_t *allocPcapHandler(const char *deviceKeywords, const char *filter, char *errBuffer);

void freePcapHandler(pcap_t **ppPcapHandler);

#endif //TRANSPORT_PCAP_HELPER_H
