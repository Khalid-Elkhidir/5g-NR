#include "ipgen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

// Compute the IPv4 header checksum.
static uint16_t ip_checksum(void *vdata, size_t length) {
    char *data = (char *)vdata;
    uint32_t acc = 0;

    // Process 16-bit words.
    for (size_t i = 0; i + 1 < length; i += 2) {
        uint16_t word;
        memcpy(&word, data + i, 2);
        acc += ntohs(word);
        if (acc > 0xffff)
            acc -= 0xffff;
    }

    // If there's a single byte left, process it.
    if (length & 1) {
        uint16_t word = 0;
        memcpy(&word, data + length - 1, 1);
        acc += ntohs(word);
        if (acc > 0xffff)
            acc -= 0xffff;
    }

    return htons(~acc & 0xffff);
}

uint8_t* generate_dummy_ip_packet(size_t *packet_size) {
    // Define a static payload.
    const char *payload = "Dummy IP Packet: Hello from the Network Layer!";
    size_t payload_len = strlen(payload);

    // IPv4 header length is 20 bytes.
    size_t header_len = sizeof(struct ip_header);
    *packet_size = header_len + payload_len;

    uint8_t *packet = malloc(*packet_size);
    if (!packet) {
        perror("malloc");
        return NULL;
    }

    struct ip_header iph;
    iph.ver_ihl = (4 << 4) | (header_len / 4); // Version 4 and IHL (in 32-bit words)
    iph.tos = 0;
    iph.total_length = htons(*packet_size);
    iph.identification = htons(0x1234); // Example ID.
    iph.flags_fragment = htons(0x4000); // Don't Fragment flag set.
    iph.ttl = 64;
    iph.protocol = 17; // UDP
    iph.checksum = 0;  // Set to 0 before computing checksum.
    iph.src_addr = inet_addr("192.168.1.100");
    iph.dest_addr = inet_addr("192.168.1.200");

    iph.checksum = ip_checksum(&iph, header_len);

    memcpy(packet, &iph, header_len);
    memcpy(packet + header_len, payload, payload_len);

    return packet;
}
