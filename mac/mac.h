#ifndef MAC_H
#define MAC_H

#include <stddef.h>
#include <stdint.h>
#include "../harq/harq.h"

/**
 * mac_dl_sch_data_transfer - Handle downlink shared channel data transfer
 * @proc: HARQ process handling the transfer
 * @received_ndi: New Data Indicator from physical layer
 * @received_rv: Redundancy Version for this transmission
 * @tb_data: Transport block data to process
 * @tb_size: Size of transport block in bytes
 *
 * Processes incoming data on the DL-SCH by managing HARQ operations
 * and forwarding successfully received data to upper layers.
 */
void mac_dl_sch_data_transfer(harq_process_t *proc, int received_ndi, int received_rv,
                              uint8_t *tb_data, size_t tb_size);

/**
 * mac_ul_sch_data_transfer - Handle uplink shared channel data transfer
 * @proc: HARQ process handling the transfer
 * @mac_pdu: MAC PDU to transmit
 * @pdu_size: Size of MAC PDU in bytes
 *
 * Manages transmission of data on the UL-SCH including HARQ
 * operations and interaction with the physical layer.
 */
void mac_ul_sch_data_transfer(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size);

/* ============================================================================
   Logical Channel Mapping and Multiplexing/Demultiplexing
   ============================================================================ */

/**
 * enum logical_channel_type_t - Types of logical channels
 * @LC_TYPE_BCCH: Broadcast Control Channel
 * @LC_TYPE_PCCH: Paging Control Channel
 * @LC_TYPE_CCCH: Common Control Channel
 * @LC_TYPE_DCCH: Dedicated Control Channel
 * @LC_TYPE_DTCH: Dedicated Traffic Channel
 *
 * Defines the different types of logical channels used for
 * various control and user plane data flows.
 */
typedef enum {
    LC_TYPE_BCCH,
    LC_TYPE_PCCH,
    LC_TYPE_CCCH,
    LC_TYPE_DCCH,
    LC_TYPE_DTCH
} logical_channel_type_t;

/**
 * struct logical_channel_t - Logical channel configuration
 * @channel_id: Unique identifier for this channel
 * @type: Type of logical channel (BCCH, PCCH, etc.)
 * @priority: Channel priority (lower number = higher priority)
 * @buffer: Pending data for transmission
 * @buffer_size: Amount of pending data in bytes
 *
 * Contains configuration and state information for a logical
 * channel, including its transmission buffer.
 */
typedef struct {
    int channel_id;
    logical_channel_type_t type;
    int priority; // Lower number indicates higher priority
    uint8_t *buffer;  // Data waiting to be transmitted
    size_t buffer_size;
} logical_channel_t;

/**
 * enum transport_channel_t - Types of transport channels
 * @TRANSPORT_BCH: Broadcast Channel
 * @TRANSPORT_PCH: Paging Channel
 * @TRANSPORT_DL_SCH: Downlink Shared Channel
 * @TRANSPORT_RACH: Random Access Channel
 * @TRANSPORT_UL_SCH: Uplink Shared Channel
 * @TRANSPORT_INVALID: Invalid/Unknown Channel Type
 *
 * Defines the different types of transport channels used
 * to carry data between MAC and physical layers.
 */
typedef enum {
    TRANSPORT_BCH,
    TRANSPORT_PCH,
    TRANSPORT_DL_SCH,
    TRANSPORT_RACH,
    TRANSPORT_UL_SCH,
    TRANSPORT_INVALID
} transport_channel_t;

/**
 * enum direction_t - Data transfer direction
 * @DIRECTION_DOWNLINK: Network to UE transmission
 * @DIRECTION_UPLINK: UE to network transmission
 *
 * Specifies the direction of data flow for channel mapping
 * and other directional operations.
 */
typedef enum {
    DIRECTION_DOWNLINK,
    DIRECTION_UPLINK
} direction_t;

/**
 * mac_map_logical_channel - Map logical to transport channel
 * @lc: Logical channel to map
 * @dir: Direction of data flow
 *
 * Maps a logical channel to appropriate transport channel
 * based on channel type and transmission direction.
 *
 * Return: Selected transport channel type
 */
transport_channel_t mac_map_logical_channel(const logical_channel_t *lc, direction_t dir);

/**
 * transport_channel_to_string - Get transport channel name
 * @tc: Transport channel type
 *
 * Converts transport channel enumeration to human-readable string.
 *
 * Return: String representation of transport channel
 */
const char* transport_channel_to_string(transport_channel_t tc);

/**
 * mac_multiplex - Combine data from multiple logical channels
 * @channels: Array of logical channels with data
 * @num_channels: Number of channels in array
 * @pdu_size: Size of resulting PDU
 *
 * Multiplexes data from multiple logical channels into a single
 * MAC PDU based on priorities and available data.
 *
 * Return: Pointer to multiplexed MAC PDU
 */
uint8_t *mac_multiplex(logical_channel_t *channels, int num_channels, size_t *pdu_size);

/**
 * mac_demultiplex - Split MAC PDU into logical channels
 * @mac_pdu: PDU to demultiplex
 * @pdu_size: Size of PDU in bytes
 *
 * Separates a MAC PDU into individual logical channel data
 * streams based on embedded multiplexing information.
 */
void mac_demultiplex(uint8_t *mac_pdu, size_t pdu_size);

/**
 * SR_THRESHOLD - Buffer threshold for scheduling request
 *
 * When buffer size exceeds this value (in bytes), a
 * scheduling request will be triggered.
 */
#define SR_THRESHOLD 50

/**
 * mac_handle_sr - Process scheduling requests
 * @channels: Array of logical channels
 * @num_channels: Number of channels in array
 *
 * Evaluates buffer status of logical channels and triggers
 * scheduling requests when needed.
 */
void mac_handle_sr(logical_channel_t *channels, int num_channels);

/**
 * mac_report_bsr - Generate buffer status report
 * @channels: Array of logical channels
 * @num_channels: Number of channels in array
 *
 * Creates and sends buffer status report to help network
 * schedule appropriate uplink resources.
 */
void mac_report_bsr(logical_channel_t *channels, int num_channels);

/* ============================================================================
   HARQ Process Interface for RLC Transmission
   ============================================================================ */

/**
 * mac_get_harq_process - Get available HARQ process
 *
 * Return: Pointer to available HARQ process
 *
 * Provides access to a HARQ process for new transmissions.
 * In a full implementation, this would manage a pool of processes.
 */
harq_process_t* mac_get_harq_process(void);

#endif // MAC_H
