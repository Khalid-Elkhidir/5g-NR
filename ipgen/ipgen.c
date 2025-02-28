#include "ipgen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

/**
 * ip_checksum - Calculate IPv4 header checksum
 * @vdata: Pointer to data to checksum
 * @length: Length of data in bytes
 *
 * Computes the Internet checksum as defined in RFC 1071.
 * Processes data as 16-bit words and handles odd-length
 * data appropriately.
 *
 * Return: 16-bit checksum in network byte order
 */
static uint16_t ip_checksum(void *vdata, size_t length) {
    char *data = (char *)vdata;
    uint32_t acc = 0;

    /* Process 16-bit words */
    for (size_t i = 0; i + 1 < length; i += 2) {
        uint16_t word;
        memcpy(&word, data + i, 2);
        acc += ntohs(word);
        if (acc > 0xffff)
            acc -= 0xffff;
    }

    /* Handle single byte at end if present */
    if (length & 1) {
        uint16_t word = 0;
        memcpy(&word, data + length - 1, 1);
        acc += ntohs(word);
        if (acc > 0xffff)
            acc -= 0xffff;
    }

    return htons(~acc & 0xffff);
}

/**
 * generate_dummy_ip_packet - Create a test IPv4 packet
 * @packet_size: Pointer to store total packet size
 *
 * Generates an IPv4 packet with:
 * - Source IP: 192.168.1.100
 * - Destination IP: 192.168.1.200
 * - Protocol: UDP (17)
 * - TTL: 64
 * - Don't Fragment flag set
 * - Test payload message
 *
 * Return: Pointer to complete packet (caller must free) or NULL on error
 */
uint8_t* generate_dummy_ip_packet(size_t *packet_size) {
    /* Create test payload */
    const char *payload = "Dummy IP Packet: Hello from the Network Layer!";
    size_t payload_len = strlen(payload);

    /* Calculate total packet size */
    size_t header_len = sizeof(struct ip_header);
    *packet_size = header_len + payload_len;

    /* Allocate packet buffer */
    uint8_t *packet = malloc(*packet_size);
    if (!packet) {
        perror("malloc");
        return NULL;
    }

    /* Initialize IPv4 header */
    struct ip_header iph;
    iph.ver_ihl = (4 << 4) | (header_len / 4);  /* IPv4 with 5 32-bit words */
    iph.tos = 0;
    iph.total_length = htons(*packet_size);
    iph.identification = htons(0x1234);          /* Example packet ID */
    iph.flags_fragment = htons(0x4000);          /* Set Don't Fragment flag */
    iph.ttl = 64;
    iph.protocol = 17;                           /* UDP protocol */
    iph.checksum = 0;                            /* Zero before checksum calculation */
    iph.src_addr = inet_addr("192.168.1.100");
    iph.dest_addr = inet_addr("192.168.1.200");

    /* Calculate header checksum */
    iph.checksum = ip_checksum(&iph, header_len);

    /* Assemble complete packet */
    memcpy(packet, &iph, header_len);
    memcpy(packet + header_len, payload, payload_len);

    return packet;
}
