#ifndef IPGEN_H
#define IPGEN_H

#include <stddef.h>
#include <stdint.h>

 // IPv4 header structure (minimal 20 bytes)
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

// Generates a valid IPv4 packet with static source ("192.168.1.100") and destination ("192.168.1.200") IPs.
// Returns a pointer to the packet (allocated dynamically) and sets *packet_size to the total size.
// The caller is responsible for freeing the returned pointer.
uint8_t* generate_dummy_ip_packet(size_t *packet_size);

#endif // IPGEN_H
