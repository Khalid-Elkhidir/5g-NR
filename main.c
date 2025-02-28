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

/**
 * global_rlc_dl_entity - Pointer to the downlink RLC entity used for loopback
 * This pointer must be set before initiating the loopback process
 */
rlc_entity_t *global_rlc_dl_entity = NULL;

int main(void) {
    printf("=== 5G NR Layer 2 Loopback Simulation ===\n");

    /* Initialize PDCP layer and get a handle to the PDCP entity */
    pdcp_entity_t *pdcp_ent = pdcp_get_entity();

    /* Create a HARQ process for handling retransmissions in MAC layer */
    harq_process_t *harq_ptr = mac_get_harq_process();

    /* Initialize downlink RLC entity in Transparent Mode for data loopback */
    rlc_entity_t rlc_dl;
    rlc_entity_establish(&rlc_dl, RLC_MODE_TM);
    global_rlc_dl_entity = &rlc_dl;

    /* Main simulation loop - processes packets continuously */
    while (1) {
        printf("\n-------------------------------\n");
        printf("Starting new packet transmission cycle...\n");

        /* Step 1: Create a test IP packet to simulate network traffic */
        size_t ip_packet_size = 0;
        uint8_t *ip_packet = generate_dummy_ip_packet(&ip_packet_size);
        if (!ip_packet) {
            printf("Error: Failed to generate dummy IP packet.\n");
            break;
        }
        printf("Network: Generated IP packet of %zu bytes.\n", ip_packet_size);
        printf("Network: Source IP = 192.168.1.100, Destination IP = 192.168.1.200\n");

        /* Step 2: Process packet through PDCP layer (header addition, security) */
        size_t pdcp_pdu_size = 0;
        uint8_t *pdcp_pdu = pdcp_prepare_tx_pdu(pdcp_ent, ip_packet, ip_packet_size, &pdcp_pdu_size);
        free(ip_packet);
        if (!pdcp_pdu) {
            printf("PDCP: Failed to prepare PDCP PDU.\n");
            break;
        }
        printf("PDCP: Prepared PDCP PDU of %zu bytes.\n", pdcp_pdu_size);

        /* Step 3: Simulate uplink transmission through RLC layer */
        rlc_entity_t rlc_tx;
        rlc_entity_establish(&rlc_tx, RLC_MODE_TM);
        printf("RLC (TX): Instantiated Transparent Mode entity for uplink transmission.\n");
        rlc_tm_tx_data(&rlc_tx, pdcp_pdu, pdcp_pdu_size);
        rlc_entity_release(&rlc_tx);
        printf("RLC (TX): Released uplink RLC entity.\n");

        /* Simulate network propagation delay */
        sleep(1);

        /* Step 4: Simulate MAC layer loopback
         * In a real system, this data would come from the physical layer
         * Here we simulate receiving the same PDU in downlink
         */
        printf("MAC: Loopback simulation triggered.\n");
        mac_loopback_pdu(harq_ptr, pdcp_pdu, pdcp_pdu_size);
        free(pdcp_pdu);

        /* Step 5: The loopback process will:
         * - Pass data to RLC downlink
         * - RLC (in transparent mode) forwards to PDCP
         * - PDCP processes the received PDU
         */

        /* Add delay between transmission cycles to control traffic rate */
        sleep(2);
    }

    /* Clean up resources before exiting */
    rlc_entity_release(&rlc_dl);
    global_rlc_dl_entity = NULL;
    printf("Simulation terminated. Cleaning up entities.\n");

    return 0;
}
