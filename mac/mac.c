#include "mac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../harq/harq.h"

/* --- HARQ Process Pool --- */
/* For simplicity we use a single static HARQ process.
   In a full system, you may allocate a pool and select one based on criteria.
*/
static harq_process_t global_harq_process;

harq_process_t* mac_get_harq_process(void) {
    // Ensure the global HARQ process is initialized (could be done once during system init)
    // Here we assume it is already initialized or we could call harq_init_process(&global_harq_process, <id>);
    return &global_harq_process;
}

/* --------------------------------------------------------------------------
   DL-SCH and UL-SCH Data Transfer Procedures
   -------------------------------------------------------------------------- */
void mac_dl_sch_data_transfer(harq_process_t *proc, int received_ndi, int received_rv,
                              uint8_t *tb_data, size_t tb_size) {
    printf("MAC: Processing DL-SCH data transfer\n");
    harq_handle_dl_assignment(proc, received_ndi, received_rv, tb_data, tb_size);
}

void mac_ul_sch_data_transfer(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size) {
    printf("MAC: Processing UL-SCH data transfer\n");
    harq_ul_start_tx(proc, mac_pdu, pdu_size);
}

/* --------------------------------------------------------------------------
   Logical Channel Multiplexing/Demultiplexing
   -------------------------------------------------------------------------- */
uint8_t *mac_multiplex(logical_channel_t *channels, int num_channels, size_t *pdu_size) {
    size_t total_size = 0;
    for (int i = 0; i < num_channels; i++) {
        if (channels[i].buffer_size > 0) {
            total_size += 3 + channels[i].buffer_size;
        }
    }
    if (total_size == 0) {
        *pdu_size = 0;
        return NULL;
    }
    uint8_t *pdu = (uint8_t *)malloc(total_size);
    if (!pdu) {
        *pdu_size = 0;
        return NULL;
    }
    size_t offset = 0;
    for (int i = 0; i < num_channels; i++) {
        if (channels[i].buffer_size > 0) {
            pdu[offset++] = (uint8_t)channels[i].channel_id;
            pdu[offset++] = (uint8_t)(channels[i].buffer_size & 0xFF);
            pdu[offset++] = (uint8_t)((channels[i].buffer_size >> 8) & 0xFF);
            memcpy(&pdu[offset], channels[i].buffer, channels[i].buffer_size);
            offset += channels[i].buffer_size;
        }
    }
    *pdu_size = total_size;
    printf("MAC Multiplex: Created MAC PDU of size %zu bytes\n", total_size);
    return pdu;
}

void mac_demultiplex(uint8_t *mac_pdu, size_t pdu_size) {
    size_t offset = 0;
    printf("MAC Demultiplex: Processing MAC PDU of size %zu bytes\n", pdu_size);
    while (offset + 3 <= pdu_size) {
        uint8_t channel_id = mac_pdu[offset++];
        size_t length = mac_pdu[offset++];
        length |= ((size_t)mac_pdu[offset++]) << 8;
        if (offset + length > pdu_size) {
            printf("MAC Demultiplex: Error - invalid length\n");
            return;
        }
        printf("  Channel ID: %d, Data Length: %zu, Data: ", channel_id, length);
        for (size_t i = 0; i < length; i++) {
            printf("%c", mac_pdu[offset + i]);
        }
        printf("\n");
        offset += length;
    }
}

/* --------------------------------------------------------------------------
   Scheduling Request (SR) and Buffer Status Reporting (BSR)
   -------------------------------------------------------------------------- */
void mac_handle_sr(logical_channel_t *channels, int num_channels) {
    int sr_triggered = 0;
    for (int i = 0; i < num_channels; i++) {
        if (channels[i].buffer_size > SR_THRESHOLD) {
            printf("MAC SR: Scheduling Request triggered for Logical Channel %d (buffer size: %zu bytes)\n",
                   channels[i].channel_id, channels[i].buffer_size);
            sr_triggered = 1;
        }
    }
    if (!sr_triggered) {
        printf("MAC SR: No Scheduling Request needed (all channel buffers below threshold)\n");
    }
}

void mac_report_bsr(logical_channel_t *channels, int num_channels) {
    printf("MAC BSR: Buffer Status Report\n");
    for (int i = 0; i < num_channels; i++) {
        printf("  Logical Channel %d: Buffer Size = %zu bytes\n",
               channels[i].channel_id, channels[i].buffer_size);
    }
}

/* --------------------------------------------------------------------------
   Logical Channel Mapping
   -------------------------------------------------------------------------- */
transport_channel_t mac_map_logical_channel(const logical_channel_t *lc, direction_t dir) {
    if (dir == DIRECTION_DOWNLINK) {
        switch (lc->type) {
            case LC_TYPE_BCCH: return TRANSPORT_BCH;
            case LC_TYPE_PCCH: return TRANSPORT_PCH;
            case LC_TYPE_CCCH:
            case LC_TYPE_DCCH:
            case LC_TYPE_DTCH: return TRANSPORT_DL_SCH;
            default: return TRANSPORT_INVALID;
        }
    } else if (dir == DIRECTION_UPLINK) {
        switch (lc->type) {
            case LC_TYPE_CCCH:
            case LC_TYPE_DCCH:
            case LC_TYPE_DTCH: return TRANSPORT_UL_SCH;
            case LC_TYPE_BCCH:
            case LC_TYPE_PCCH: return TRANSPORT_INVALID;
            default: return TRANSPORT_INVALID;
        }
    }
    return TRANSPORT_INVALID;
}

const char* transport_channel_to_string(transport_channel_t tc) {
    switch (tc) {
        case TRANSPORT_BCH: return "BCH";
        case TRANSPORT_PCH: return "PCH";
        case TRANSPORT_DL_SCH: return "DL-SCH";
        case TRANSPORT_RACH: return "RACH";
        case TRANSPORT_UL_SCH: return "UL-SCH";
        default: return "INVALID";
    }
}
