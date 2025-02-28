#include "rlc.h"
#include "../pdcp/pdcp.h"
#include "../mac/mac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------- Entity Handling -------------------- */
void rlc_entity_establish(rlc_entity_t *entity, rlc_mode_t mode) {
    if (!entity) return;
    entity->mode = mode;
    entity->tx_next = 0;
    entity->rx_next = 0;
    entity->reassembly_buffer = NULL;
    entity->reassembly_size = 0;
    entity->reassembly_sn = 0;
    printf("RLC: Entity established in mode %d\n", mode);
}

void rlc_entity_reestablish(rlc_entity_t *entity) {
    if (!entity) return;
    entity->tx_next = 0;
    entity->rx_next = 0;
    if (entity->reassembly_buffer) {
        free(entity->reassembly_buffer);
        entity->reassembly_buffer = NULL;
    }
    entity->reassembly_size = 0;
    entity->reassembly_sn = 0;
    printf("RLC: Entity re-established\n");
}

void rlc_entity_release(rlc_entity_t *entity) {
    if (!entity) return;
    if (entity->reassembly_buffer) {
        free(entity->reassembly_buffer);
        entity->reassembly_buffer = NULL;
    }
    printf("RLC: Entity released\n");
}

/* -------------------- Transparent Mode (TM) -------------------- */
/* In TM mode the PDCP Data PDU (which already includes the PDCP header)
   is passed unchanged as a TMD PDU to the MAC sublayer.
*/
void rlc_tm_tx_data(rlc_entity_t *entity, uint8_t *pdcp_pdu, size_t pdu_size) {
    printf("RLC TM: Transmitting PDCP PDU of size %zu bytes\n", pdu_size);
    harq_process_t *harq_ptr = mac_get_harq_process();
    mac_ul_sch_data_transfer(harq_ptr, pdcp_pdu, pdu_size);
}

void rlc_tm_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size) {
    printf("RLC TM: Received PDCP PDU of size %zu bytes\n", pdu_size);
    // Obtain the PDCP entity from the PDCP sublayer.
    pdcp_entity_t *pdcp_ent = pdcp_get_entity();
    pdcp_rx_pdu(pdcp_ent, pdu, pdu_size);
}

/* -------------------- Unacknowledged Mode (UM) -------------------- */
/* In UM mode the PDCP Data PDU (including its header) is segmented if needed.
   For the first segment the PDCP header is included; for subsequent segments,
   only the payload is sent. On reception, segmented PDUs are reassembled.
*/
void rlc_um_tx_data(rlc_entity_t *entity, uint8_t *pdcp_pdu, size_t pdu_size) {
    if (!entity || !pdcp_pdu) return;
    printf("RLC UM: Transmitting PDCP PDU of size %zu bytes\n", pdu_size);
    harq_process_t *harq_ptr = mac_get_harq_process();
    if (pdu_size <= RLC_UM_SEGMENT_SIZE) {
        uint8_t header[2];
        header[0] = entity->tx_next; // SN
        header[1] = 0;               // SI = 0 (complete)
        size_t um_pdu_size = 2 + pdu_size;
        uint8_t *um_pdu = (uint8_t *)malloc(um_pdu_size);
        if (!um_pdu) return;
        memcpy(um_pdu, header, 2);
        memcpy(um_pdu + 2, pdcp_pdu, pdu_size);
        mac_ul_sch_data_transfer(harq_ptr, um_pdu, um_pdu_size);
        free(um_pdu);
    } else {
        size_t remaining = pdu_size;
        size_t offset = 0;
        while (remaining > 0) {
            size_t seg_size = (remaining > RLC_UM_SEGMENT_SIZE) ? RLC_UM_SEGMENT_SIZE : remaining;
            uint8_t si;
            if (offset == 0 && remaining > RLC_UM_SEGMENT_SIZE) {
                si = 1; // first segment (includes PDCP header)
            } else if (remaining <= RLC_UM_SEGMENT_SIZE) {
                si = (offset == 0) ? 0 : 3; // if only one segment, SI=0; otherwise last segment=3
            } else {
                si = 2; // middle segment
            }
            size_t header_size = (offset == 0) ? 2 : 4;
            size_t um_pdu_size = header_size + seg_size;
            uint8_t *um_pdu = (uint8_t *)malloc(um_pdu_size);
            if (!um_pdu) return;
            um_pdu[0] = entity->tx_next; // SN
            um_pdu[1] = si;
            if (offset != 0) {
                um_pdu[2] = (offset >> 8) & 0xFF;
                um_pdu[3] = offset & 0xFF;
            }
            memcpy(um_pdu + header_size, pdcp_pdu + offset, seg_size);
            printf("RLC UM: Transmitting segment: SN=%d, SI=%d, offset=%zu, segment size=%zu\n",
                   entity->tx_next, si, offset, seg_size);
            mac_ul_sch_data_transfer(harq_ptr, um_pdu, um_pdu_size);
            free(um_pdu);
            offset += seg_size;
            remaining -= seg_size;
        }
    }
    entity->tx_next++;  // Increment SN for UM mode
}

void rlc_um_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size) {
    if (!entity || !pdu) return;
    if (pdu_size < 2) {
        printf("RLC UM: Invalid PDU size\n");
        return;
    }
    uint8_t sn = pdu[0];
    uint8_t si = pdu[1];
    size_t header_size = (si == 0 || si == 1) ? 2 : 4;
    if (pdu_size < header_size) {
        printf("RLC UM: PDU too short for header\n");
        return;
    }
    size_t data_size = pdu_size - header_size;
    if (si == 0) {
        printf("RLC UM: Received complete PDCP PDU (SN=%d) of size %zu bytes\n", sn, data_size);
        pdcp_rx_pdu(pdcp_get_entity(), pdu, pdu_size);
    } else {
        uint16_t so = 0;
        if (header_size == 4) {
            so = (pdu[2] << 8) | pdu[3];
        }
        printf("RLC UM: Received segmented PDU (SN=%d, SI=%d, SO=%d)\n", sn, si, so);
        if (entity->reassembly_buffer == NULL || entity->reassembly_sn != sn) {
            if (entity->reassembly_buffer)
                free(entity->reassembly_buffer);
            entity->reassembly_buffer = NULL;
            entity->reassembly_size = 0;
            entity->reassembly_sn = sn;
        }
        uint8_t *new_buf = realloc(entity->reassembly_buffer, entity->reassembly_size + data_size);
        if (!new_buf) {
            printf("RLC UM: Reassembly buffer allocation error\n");
            return;
        }
        entity->reassembly_buffer = new_buf;
        memcpy(entity->reassembly_buffer + entity->reassembly_size, pdu + header_size, data_size);
        entity->reassembly_size += data_size;
        if (si == 3) {
            printf("RLC UM: Reassembled PDCP PDU (SN=%d) of size %zu bytes\n", sn, entity->reassembly_size);
            pdcp_rx_pdu(pdcp_get_entity(), entity->reassembly_buffer, entity->reassembly_size);
            free(entity->reassembly_buffer);
            entity->reassembly_buffer = NULL;
            entity->reassembly_size = 0;
        }
    }
}
