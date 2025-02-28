#include "loopback.h"
#include "../rlc/rlc.h"
#include <stdio.h>

/* 
 * global_rlc_dl_entity:
 * This external pointer must be set by the MAC sublayer or the main application
 * to point to the already-established downlink RLC entity.
 */
extern rlc_entity_t *global_rlc_dl_entity;

void mac_loopback_pdu(harq_process_t *harq, uint8_t *pdu, size_t pdu_size) {
    printf("MAC Loopback: Received MAC PDU from PHY (size: %zu bytes).\n", pdu_size);
    if (global_rlc_dl_entity == NULL) {
        printf("MAC Loopback: Error – no downlink RLC entity available.\n");
        return;
    }
    /* Instead of creating a new RLC entity, use the existing downlink RLC entity
       to process the received MAC PDU. The HARQ process (if needed) is passed along.
       Here, we assume that in Transparent Mode the RLC downlink receive function (rlc_tm_rx_data)
       will forward the PDU upward (to PDCP) as appropriate.
    */
    rlc_tm_rx_data(global_rlc_dl_entity, pdu, pdu_size);
}
