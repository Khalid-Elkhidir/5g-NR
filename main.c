#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "harq/harq.h"
#include "mac/mac.h"
#include "loopback/loopback.h"
#include "rlc/rlc.h"
#include "pdcp/pdcp.h"
#include "ipgen/ipgen.h"

// Global downlink RLC entity pointer for loopback (set before loopback)
rlc_entity_t *global_rlc_dl_entity = NULL;

int main(void) {
    printf("=== 5G NR Layer 2 Loopback Simulation ===\n");

    // Obtain a global PDCP entity (initializes PDCP if not already done)
    pdcp_entity_t *pdcp_ent = pdcp_get_entity();

    // Create a HARQ process for MAC transmission (used in both uplink and loopback)
    harq_process_t *harq_ptr = mac_get_harq_process();

    // Set up a downlink RLC entity in Transparent Mode for loopback.
    rlc_entity_t rlc_dl;
    rlc_entity_establish(&rlc_dl, RLC_MODE_TM);
    global_rlc_dl_entity = &rlc_dl;

    // Main processing loop.
    while (1) {
        printf("\n-------------------------------\n");
        printf("Starting new packet transmission cycle...\n");

        // Step 1: Generate a dummy IP packet.
        size_t ip_packet_size = 0;
        uint8_t *ip_packet = generate_dummy_ip_packet(&ip_packet_size);
        if (!ip_packet) {
            printf("Error: Failed to generate dummy IP packet.\n");
            break;
        }
        printf("Network: Generated IP packet of %zu bytes.\n", ip_packet_size);
        // For demonstration, print static IP info.
        printf("Network: Source IP = 192.168.1.100, Destination IP = 192.168.1.200\n");

        // Step 2: PDCP processing.
        size_t pdcp_pdu_size = 0;
        uint8_t *pdcp_pdu = pdcp_prepare_tx_pdu(pdcp_ent, ip_packet, ip_packet_size, &pdcp_pdu_size);
        free(ip_packet);
        if (!pdcp_pdu) {
            printf("PDCP: Failed to prepare PDCP PDU.\n");
            break;
        }
        printf("PDCP: Prepared PDCP PDU of %zu bytes.\n", pdcp_pdu_size);

        // Step 3: RLC transmission (uplink simulation).
        rlc_entity_t rlc_tx;
        rlc_entity_establish(&rlc_tx, RLC_MODE_TM);
        printf("RLC (TX): Instantiated Transparent Mode entity for uplink transmission.\n");
        rlc_tm_tx_data(&rlc_tx, pdcp_pdu, pdcp_pdu_size);
        rlc_entity_release(&rlc_tx);
        printf("RLC (TX): Released uplink RLC entity.\n");

        // (Simulate delay between uplink and loopback.)
        sleep(1);

        // Step 4: MAC loopback simulation.
        // In a real system, the MAC sublayer would receive the PDU from the PHY.
        // Here we call mac_loopback_pdu() with the same PDCP PDU to simulate a downlink.
        printf("MAC: Loopback simulation triggered.\n");
        mac_loopback_pdu(harq_ptr, pdcp_pdu, pdcp_pdu_size);
        free(pdcp_pdu);

        // Step 5: (Inside loopback, the RLC downlink receive function is called,
        // which in Transparent Mode simply forwards the PDU to PDCP. PDCP then processes it.)
        // End of one complete chain.

        // Optional: Delay between cycles to simulate traffic rate.
        sleep(2);
    }

    // Cleanup the global downlink RLC entity.
    rlc_entity_release(&rlc_dl);
    global_rlc_dl_entity = NULL;
    printf("Simulation terminated. Cleaning up entities.\n");

    return 0;
}
