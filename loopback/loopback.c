#include "loopback.h"
#include "../rlc/rlc.h"
#include <stdio.h>

/**
 * global_rlc_dl_entity - Global downlink RLC entity pointer
 *
 * This pointer must be initialized by the MAC layer or main
 * application before any loopback operations can occur.
 * Points to a pre-established downlink RLC entity.
 */
extern rlc_entity_t *global_rlc_dl_entity;

/**
 * mac_loopback_pdu - Process loopback of MAC PDU
 * @harq: HARQ process associated with original transmission
 * @pdu: MAC PDU data to loopback
 * @pdu_size: Size of MAC PDU in bytes
 *
 * Simulates physical layer loopback by:
 * 1. Verifying downlink RLC entity is available
 * 2. Forwarding PDU to RLC layer in transparent mode
 * 3. Allowing RLC to process and forward to PDCP
 *
 * Uses existing downlink RLC entity rather than creating
 * a new one for each loopback operation.
 */
void mac_loopback_pdu(harq_process_t *harq, uint8_t *pdu, size_t pdu_size) {
    printf("MAC Loopback: Received MAC PDU from PHY (size: %zu bytes).\n", pdu_size);
    if (global_rlc_dl_entity == NULL) {
        printf("MAC Loopback: Error – no downlink RLC entity available.\n");
        return;
    }

    /* Forward PDU to RLC layer for transparent mode processing */
    rlc_tm_rx_data(global_rlc_dl_entity, pdu, pdu_size);
}
