#ifndef IPGEN_H
#define IPGEN_H

#include <stddef.h>
#include <stdint.h>

/**
 * struct ip_header - IPv4 header structure (20 bytes)
 * @ver_ihl: Version (4 bits) + Internet Header Length (4 bits)
 * @tos: Type of Service field
 * @total_length: Total packet length (header + payload)
 * @identification: Packet identification field
 * @flags_fragment: Flags (3 bits) + Fragment offset (13 bits)
 * @ttl: Time To Live value
 * @protocol: Protocol identifier (e.g., 17 for UDP)
 * @checksum: Header checksum field
 * @src_addr: Source IP address
 * @dest_addr: Destination IP address
 *
 * Defines the structure of an IPv4 header with all required
 * fields as specified in RFC 791. The structure is packed
 * to ensure proper byte alignment.
 */
struct ip_header {
    uint8_t ver_ihl;       // Version (4 bits) + Internet Header Length (4 bits)
    uint8_t tos;           // Type of Service
    uint16_t total_length; // Total length of the packet (header + payload)
    uint16_t identification; // Identification field
    uint16_t flags_fragment; // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t ttl;           // Time To Live
    uint8_t protocol;      // Protocol (e.g., 17 for UDP)
    uint16_t checksum;     // Header checksum
    uint32_t src_addr;     // Source IP address
    uint32_t dest_addr;    // Destination IP address
} __attribute__((packed));

/**
 * generate_dummy_ip_packet - Create a test IPv4 packet
 * @packet_size: Pointer to store the total packet size
 *
 * Generates an IPv4 packet with static source (192.168.1.100)
 * and destination (192.168.1.200) addresses. The packet includes
 * a valid header and dummy payload data.
 *
 * Return: Pointer to the generated packet (caller must free)
 */
uint8_t* generate_dummy_ip_packet(size_t *packet_size);

#endif // IPGEN_H
