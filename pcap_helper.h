//
// Created by tyran on 2024/8/19.
//

#ifndef TRANSPORT_PCAP_HELPER_H
#define TRANSPORT_PCAP_HELPER_H

#include <pcap.h>

pcap_t *allocPcapHandler(const char *deviceKeywords, char *errBuffer);

void freePcapHandler(pcap_t **ppPcapHandler);

#endif //TRANSPORT_PCAP_HELPER_H
