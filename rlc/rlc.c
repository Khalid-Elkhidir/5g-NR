#include "rlc.h"
#include "../pdcp/pdcp.h"
#include "../mac/mac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * rlc_entity_establish - Initialize a new RLC entity
 * @entity: Pointer to RLC entity to initialize
 * @mode: Operating mode for this entity
 *
 * Creates a new RLC entity with specified mode and initializes
 * all state variables and buffers to their default values.
 * Prints confirmation message upon successful establishment.
 */
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

/**
 * rlc_entity_reestablish - Reset an existing RLC entity
 * @entity: Pointer to RLC entity to reset
 *
 * Resets all sequence numbers and clears buffers while maintaining
 * the entity's operational mode. Used after radio link failure
 * or during handover procedures.
 */
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

/**
 * rlc_entity_release - Clean up an RLC entity
 * @entity: Pointer to RLC entity to release
 *
 * Frees all allocated resources and resets the entity state.
 * Should be called when the RLC entity is no longer needed.
 */
void rlc_entity_release(rlc_entity_t *entity) {
    if (!entity) return;
    if (entity->reassembly_buffer) {
        free(entity->reassembly_buffer);
        entity->reassembly_buffer = NULL;
    }
    printf("RLC: Entity released\n");
}

/* Transparent Mode (TM) Operations */

/**
 * rlc_tm_tx_data - Transmit data in Transparent Mode
 * @entity: RLC entity handling the transmission
 * @pdcp_pdu: PDCP PDU to transmit (includes PDCP header)
 * @pdu_size: Size of the PDCP PDU in bytes
 *
 * In TM mode, forwards PDCP PDU directly to MAC layer without
 * any additional processing or headers. The PDCP header remains
 * intact as part of the transmitted data.
 */
void rlc_tm_tx_data(rlc_entity_t *entity, uint8_t *pdcp_pdu, size_t pdu_size) {
    printf("RLC TM: Transmitting PDCP PDU of size %zu bytes\n", pdu_size);
    harq_process_t *harq_ptr = mac_get_harq_process();
    mac_ul_sch_data_transfer(harq_ptr, pdcp_pdu, pdu_size);
}

/**
 * rlc_tm_rx_data - Receive data in Transparent Mode
 * @entity: RLC entity handling the reception
 * @pdu: Received PDU data
 * @pdu_size: Size of received PDU in bytes
 *
 * Processes received data in TM mode by forwarding it directly
 * to the PDCP layer without any processing. The PDU includes
 * the original PDCP header.
 */
void rlc_tm_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size) {
    printf("RLC TM: Received PDCP PDU of size %zu bytes\n", pdu_size);
    pdcp_entity_t *pdcp_ent = pdcp_get_entity();
    pdcp_rx_pdu(pdcp_ent, pdu, pdu_size);
}

/* Unacknowledged Mode (UM) Operations */

/**
 * rlc_um_tx_data - Transmit data in Unacknowledged Mode
 * @entity: RLC entity handling the transmission
 * @pdcp_pdu: PDCP PDU to transmit
 * @pdu_size: Size of the PDCP PDU in bytes
 *
 * Handles UM mode transmission by:
 * 1. Adding RLC UM header (SN and SI fields)
 * 2. Segmenting large PDUs if needed
 * 3. Managing sequence numbers
 * 4. Forwarding segments to MAC layer
 *
 * For segmented PDUs:
 * - First segment (SI=1): Includes PDCP header
 * - Middle segments (SI=2): Only payload
 * - Last segment (SI=3): Final portion of payload
 * - Single segment (SI=0): Complete PDU with header
 */
void rlc_um_tx_data(rlc_entity_t *entity, uint8_t *pdcp_pdu, size_t pdu_size) {
    if (!entity || !pdcp_pdu) return;
    printf("RLC UM: Transmitting PDCP PDU of size %zu bytes\n", pdu_size);
    harq_process_t *harq_ptr = mac_get_harq_process();

    /* Handle PDU that fits in single segment */
    if (pdu_size <= RLC_UM_SEGMENT_SIZE) {
        uint8_t header[2];
        header[0] = entity->tx_next;  /* Sequence Number */
        header[1] = 0;                /* SI=0: Complete PDU */
        size_t um_pdu_size = 2 + pdu_size;
        uint8_t *um_pdu = (uint8_t *)malloc(um_pdu_size);
        if (!um_pdu) return;
        memcpy(um_pdu, header, 2);
        memcpy(um_pdu + 2, pdcp_pdu, pdu_size);
        mac_ul_sch_data_transfer(harq_ptr, um_pdu, um_pdu_size);
        free(um_pdu);
    } else {
        /* Handle PDU that requires segmentation */
        size_t remaining = pdu_size;
        size_t offset = 0;
        while (remaining > 0) {
            size_t seg_size = (remaining > RLC_UM_SEGMENT_SIZE) ? RLC_UM_SEGMENT_SIZE : remaining;
            uint8_t si;
            /* Determine Segment Indicator (SI) value */
            if (offset == 0 && remaining > RLC_UM_SEGMENT_SIZE) {
                si = 1;  /* First segment */
            } else if (remaining <= RLC_UM_SEGMENT_SIZE) {
                si = (offset == 0) ? 0 : 3;  /* Last segment or complete PDU */
            } else {
                si = 2;  /* Middle segment */
            }

            /* Create and send segment with appropriate header */
            size_t header_size = (offset == 0) ? 2 : 4;
            size_t um_pdu_size = header_size + seg_size;
            uint8_t *um_pdu = (uint8_t *)malloc(um_pdu_size);
            if (!um_pdu) return;

            /* Build segment header */
            um_pdu[0] = entity->tx_next;  /* Sequence Number */
            um_pdu[1] = si;
            if (offset != 0) {
                um_pdu[2] = (offset >> 8) & 0xFF;  /* Segment Offset (high byte) */
                um_pdu[3] = offset & 0xFF;         /* Segment Offset (low byte) */
            }

            /* Add segment payload and send */
            memcpy(um_pdu + header_size, pdcp_pdu + offset, seg_size);
            printf("RLC UM: Transmitting segment: SN=%d, SI=%d, offset=%zu, segment size=%zu\n",
                   entity->tx_next, si, offset, seg_size);
            mac_ul_sch_data_transfer(harq_ptr, um_pdu, um_pdu_size);
            free(um_pdu);

            /* Update segment tracking */
            offset += seg_size;
            remaining -= seg_size;
        }
    }
    entity->tx_next++;  /* Increment sequence number for next transmission */
}

/**
 * rlc_um_rx_data - Receive data in Unacknowledged Mode
 * @entity: RLC entity handling the reception
 * @pdu: Received PDU data
 * @pdu_size: Size of received PDU in bytes
 *
 * Processes received UM mode PDUs by:
 * 1. Extracting RLC header information (SN, SI)
 * 2. Handling complete PDUs directly
 * 3. Managing reassembly of segmented PDUs
 * 4. Delivering complete PDUs to PDCP layer
 *
 * Segmentation handling:
 * - Complete PDU (SI=0): Direct delivery
 * - First segment (SI=1): Start reassembly
 * - Middle segment (SI=2): Continue reassembly
 * - Last segment (SI=3): Complete reassembly and deliver
 */
void rlc_um_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size) {
    if (!entity || !pdu) return;
    if (pdu_size < 2) {
        printf("RLC UM: Invalid PDU size\n");
        return;
    }

    /* Extract header information */
    uint8_t sn = pdu[0];
    uint8_t si = pdu[1];
    size_t header_size = (si == 0 || si == 1) ? 2 : 4;
    if (pdu_size < header_size) {
        printf("RLC UM: PDU too short for header\n");
        return;
    }

    /* Process PDU based on segmentation indicator */
    size_t data_size = pdu_size - header_size;
    if (si == 0) {
        /* Handle complete PDU */
        printf("RLC UM: Received complete PDCP PDU (SN=%d) of size %zu bytes\n", sn, data_size);
        pdcp_rx_pdu(pdcp_get_entity(), pdu, pdu_size);
    } else {
        /* Handle segmented PDU */
        uint16_t so = 0;
        if (header_size == 4) {
            so = (pdu[2] << 8) | pdu[3];  /* Extract segment offset */
        }
        printf("RLC UM: Received segmented PDU (SN=%d, SI=%d, SO=%d)\n", sn, si, so);

        /* Initialize or reset reassembly buffer if needed */
        if (entity->reassembly_buffer == NULL || entity->reassembly_sn != sn) {
            if (entity->reassembly_buffer)
                free(entity->reassembly_buffer);
            entity->reassembly_buffer = NULL;
            entity->reassembly_size = 0;
            entity->reassembly_sn = sn;
        }

        /* Add segment to reassembly buffer */
        uint8_t *new_buf = realloc(entity->reassembly_buffer, entity->reassembly_size + data_size);
        if (!new_buf) {
            printf("RLC UM: Reassembly buffer allocation error\n");
            return;
        }
        entity->reassembly_buffer = new_buf;
        memcpy(entity->reassembly_buffer + entity->reassembly_size, pdu + header_size, data_size);
        entity->reassembly_size += data_size;

        /* If last segment, deliver complete PDU */
        if (si == 3) {
            printf("RLC UM: Reassembled PDCP PDU (SN=%d) of size %zu bytes\n", sn, entity->reassembly_size);
            pdcp_rx_pdu(pdcp_get_entity(), entity->reassembly_buffer, entity->reassembly_size);
            free(entity->reassembly_buffer);
            entity->reassembly_buffer = NULL;
            entity->reassembly_size = 0;
        }
    }
}
