#ifndef MAC_H
#define MAC_H

#include <stddef.h>
#include <stdint.h>
#include "../harq/harq.h"

/* 
 * DL-SCH Data Transfer Procedure:
 */
void mac_dl_sch_data_transfer(harq_process_t *proc, int received_ndi, int received_rv,
                              uint8_t *tb_data, size_t tb_size);

/*
 * UL-SCH Data Transfer Procedure:
 */
void mac_ul_sch_data_transfer(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size);

/* ============================================================================
   Logical Channel Mapping and Multiplexing/Demultiplexing
   ============================================================================ */

typedef enum {
    LC_TYPE_BCCH,
    LC_TYPE_PCCH,
    LC_TYPE_CCCH,
    LC_TYPE_DCCH,
    LC_TYPE_DTCH
} logical_channel_type_t;

typedef struct {
    int channel_id;
    logical_channel_type_t type;
    int priority; // Lower number indicates higher priority
    uint8_t *buffer;  // Data waiting to be transmitted
    size_t buffer_size;
} logical_channel_t;

typedef enum {
    TRANSPORT_BCH,
    TRANSPORT_PCH,
    TRANSPORT_DL_SCH,
    TRANSPORT_RACH,
    TRANSPORT_UL_SCH,
    TRANSPORT_INVALID
} transport_channel_t;

typedef enum {
    DIRECTION_DOWNLINK,
    DIRECTION_UPLINK
} direction_t;

transport_channel_t mac_map_logical_channel(const logical_channel_t *lc, direction_t dir);
const char* transport_channel_to_string(transport_channel_t tc);

uint8_t *mac_multiplex(logical_channel_t *channels, int num_channels, size_t *pdu_size);
void mac_demultiplex(uint8_t *mac_pdu, size_t pdu_size);

#define SR_THRESHOLD 50
void mac_handle_sr(logical_channel_t *channels, int num_channels);
void mac_report_bsr(logical_channel_t *channels, int num_channels);

/* ============================================================================
   HARQ Process Interface for RLC Transmission
   ============================================================================ */

/* Returns a pointer to a HARQ process that can be used for transmissions.
   In a complete implementation this might select a process from a pool.
*/
harq_process_t* mac_get_harq_process(void);

#endif // MAC_H
