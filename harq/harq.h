#ifndef HARQ_H
#define HARQ_H

#include <stddef.h>
#include <stdint.h>

/**
 * enum harq_state_t - Possible states of a HARQ process
 * @HARQ_IDLE: Process is available for new transmissions
 * @HARQ_ACTIVE: Process is currently handling a transmission
 * @HARQ_WAIT_ACK: Process is waiting for acknowledgment
 */
typedef enum {
    HARQ_IDLE,
    HARQ_ACTIVE,
    HARQ_WAIT_ACK
} harq_state_t;

/**
 * struct harq_process_t - Represents a single HARQ process instance
 * @process_id: Unique identifier for this HARQ process
 * @state: Current operational state of the process
 * @ndi: New Data Indicator - toggles for fresh transmissions
 * @rv: Redundancy Version - indicates which version of data is being transmitted
 * @tb_data: Pointer to the current transport block data
 * @tb_size: Size of the transport block in bytes
 * @num_retx: Counter for number of retransmission attempts
 * @soft_buffer: Storage for combining multiple transmissions of same data
 *
 * This structure maintains all necessary state information for
 * handling hybrid ARQ operations in 5G NR.
 */
typedef struct {
    int process_id;
    harq_state_t state;
    int ndi;
    int rv;
    uint8_t *tb_data;
    size_t tb_size;
    int num_retx;
    uint8_t *soft_buffer;
} harq_process_t;

/**
 * harq_init_process - Initialize a new HARQ process
 * @proc: Pointer to the HARQ process structure
 * @process_id: Unique identifier to assign to this process
 *
 * Sets up a new HARQ process with default values and allocates
 * necessary resources.
 */
void harq_init_process(harq_process_t *proc, int process_id);

/* Downlink HARQ Functions */

/**
 * harq_handle_dl_assignment - Process a new downlink transmission
 * @proc: Target HARQ process
 * @received_ndi: New Data Indicator from physical layer
 * @received_rv: Redundancy Version for this transmission
 * @tb_data: Transport block data received
 * @tb_size: Size of the transport block
 *
 * Handles incoming downlink data, determining if it's a new transmission
 * or retransmission based on NDI and RV values.
 */
void harq_handle_dl_assignment(harq_process_t *proc, int received_ndi, int received_rv,
                             uint8_t *tb_data, size_t tb_size);

/**
 * harq_dl_process_feedback - Handle downlink acknowledgment
 * @proc: Target HARQ process
 * @ack: 1 for successful decode, 0 for failed decode
 *
 * Processes ACK/NACK feedback for downlink transmissions and
 * manages retransmission if needed.
 */
void harq_dl_process_feedback(harq_process_t *proc, int ack);

/* Uplink HARQ Functions */

/**
 * harq_ul_start_tx - Initiate an uplink transmission
 * @proc: Target HARQ process
 * @mac_pdu: MAC PDU data to transmit
 * @pdu_size: Size of the MAC PDU
 *
 * Prepares and starts a new uplink transmission by storing the
 * PDU and triggering physical layer transmission.
 */
void harq_ul_start_tx(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size);

/**
 * harq_ul_process_feedback - Handle uplink acknowledgment
 * @proc: Target HARQ process
 * @ack: 1 for successful transmission, 0 for failed transmission
 *
 * Processes feedback for uplink transmissions and schedules
 * retransmission if necessary.
 */
void harq_ul_process_feedback(harq_process_t *proc, int ack);

/* Physical Layer Interface Functions */

/**
 * phy_transmit_dl - Send data to physical layer for downlink
 * @proc: HARQ process containing data to transmit
 *
 * Interface function for triggering physical layer transmission
 * of downlink transport blocks.
 */
void phy_transmit_dl(harq_process_t *proc);

/**
 * phy_combine_dl - Combine new data with stored soft bits
 * @proc: HARQ process for combining
 * @new_data: Recently received transport block
 * @new_data_size: Size of the new transport block
 *
 * Combines newly received data with previously stored soft bits
 * for improved decoding probability.
 */
void phy_combine_dl(harq_process_t *proc, uint8_t *new_data, size_t new_data_size);

/**
 * phy_transmit_ul - Send data to physical layer for uplink
 * @proc: HARQ process containing data to transmit
 *
 * Interface function for triggering physical layer transmission
 * of uplink transport blocks.
 */
void phy_transmit_ul(harq_process_t *proc);

/**
 * rlc_deliver_mac_pdu - Forward decoded PDU to RLC layer
 * @mac_pdu: Successfully decoded MAC PDU
 * @pdu_size: Size of the MAC PDU
 *
 * Delivers successfully decoded MAC PDUs to the RLC sublayer
 * for further processing.
 */
void rlc_deliver_mac_pdu(uint8_t *mac_pdu, size_t pdu_size);

#endif /* HARQ_H */
