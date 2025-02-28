#ifndef RLC_H
#define RLC_H

#include <stddef.h>
#include <stdint.h>
#include "../mac/mac.h"   /* RLC uses MAC interface for data transmission */

/**
 * enum rlc_mode_t - Operating modes for RLC entities
 * @RLC_MODE_TM: Transparent Mode - no header, no segmentation
 * @RLC_MODE_UM: Unacknowledged Mode - with segmentation, no retransmission
 * @RLC_MODE_AM: Acknowledged Mode - with segmentation and retransmission
 *
 * Defines the three possible operational modes for RLC entities
 * as specified in 3GPP standards.
 */
typedef enum {
    RLC_MODE_TM,
    RLC_MODE_UM,
    RLC_MODE_AM
} rlc_mode_t;

/**
 * struct rlc_entity_t - RLC protocol entity instance
 * @mode: Current operational mode (TM/UM/AM)
 * @tx_next: Sequence number for next transmission in UM/AM modes
 * @rx_next: Expected sequence number for next reception in UM/AM modes
 * @reassembly_buffer: Storage for reassembling segmented SDUs
 * @reassembly_size: Current size of data in reassembly buffer
 * @reassembly_sn: Sequence number of SDU being reassembled
 *
 * Maintains the state of an RLC entity including buffers and
 * sequence numbers for segmentation/reassembly operations.
 */
typedef struct {
    rlc_mode_t mode;
    uint8_t tx_next;
    uint8_t rx_next;
    uint8_t *reassembly_buffer;
    size_t reassembly_size;
    uint8_t reassembly_sn;
} rlc_entity_t;

/* RLC Entity Management Functions */

/**
 * rlc_entity_establish - Create and initialize an RLC entity
 * @entity: Pointer to RLC entity structure to initialize
 * @mode: Operational mode to configure (TM/UM/AM)
 *
 * Sets up a new RLC entity with specified mode and initializes
 * all state variables and buffers.
 */
void rlc_entity_establish(rlc_entity_t *entity, rlc_mode_t mode);

/**
 * rlc_entity_reestablish - Reset an RLC entity
 * @entity: Pointer to RLC entity to reset
 *
 * Resets the RLC entity state while maintaining its mode,
 * typically used after radio link failure.
 */
void rlc_entity_reestablish(rlc_entity_t *entity);

/**
 * rlc_entity_release - Clean up an RLC entity
 * @entity: Pointer to RLC entity to release
 *
 * Frees all resources associated with the RLC entity
 * and resets its state variables.
 */
void rlc_entity_release(rlc_entity_t *entity);

/* Transparent Mode (TM) Functions */

/**
 * rlc_tm_tx_data - Transmit data in Transparent Mode
 * @entity: Pointer to RLC entity
 * @sdu: Data to transmit
 * @sdu_size: Size of data in bytes
 *
 * Handles transmission in TM mode - directly forwards data
 * without adding headers or performing segmentation.
 */
void rlc_tm_tx_data(rlc_entity_t *entity, uint8_t *sdu, size_t sdu_size);

/**
 * rlc_tm_rx_data - Receive data in Transparent Mode
 * @entity: Pointer to RLC entity
 * @pdu: Received data
 * @pdu_size: Size of received data in bytes
 *
 * Processes received data in TM mode - directly forwards
 * to upper layer without any processing.
 */
void rlc_tm_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size);

/* Unacknowledged Mode (UM) Functions */

/**
 * RLC_UM_SEGMENT_SIZE - Maximum size of UM mode segments
 *
 * Defines the maximum PDU size for segmentation in UM mode.
 * Larger SDUs will be split into multiple segments of this size.
 */
#define RLC_UM_SEGMENT_SIZE 20

/**
 * rlc_um_tx_data - Transmit data in Unacknowledged Mode
 * @entity: Pointer to RLC entity
 * @sdu: Data to transmit
 * @sdu_size: Size of data in bytes
 *
 * Handles transmission in UM mode including segmentation
 * and header addition for each segment.
 */
void rlc_um_tx_data(rlc_entity_t *entity, uint8_t *sdu, size_t sdu_size);

/**
 * rlc_um_rx_data - Receive data in Unacknowledged Mode
 * @entity: Pointer to RLC entity
 * @pdu: Received data
 * @pdu_size: Size of received data in bytes
 *
 * Processes received data in UM mode including reassembly
 * of segmented SDUs and in-sequence delivery.
 */
void rlc_um_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size);

#endif /* RLC_H */
