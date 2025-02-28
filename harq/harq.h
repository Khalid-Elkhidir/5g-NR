#ifndef HARQ_H
#define HARQ_H

#include <stddef.h>
#include <stdint.h>

/* HARQ state definitions */
typedef enum {
    HARQ_IDLE,
    HARQ_ACTIVE,
    HARQ_WAIT_ACK
} harq_state_t;

/* HARQ process structure.
   It holds the necessary parameters for a single HARQ process.
   - process_id: identifies the HARQ process.
   - ndi: new data indicator (toggles for new transmissions).
   - rv: redundancy version (indicates the transmission instance).
   - tb_data: pointer to the current transport block data.
   - tb_size: size (in bytes) of the transport block.
   - num_retx: number of retransmissions attempted.
   - soft_buffer: buffer for soft combining of retransmissions.
   - state: current state of the HARQ process.
*/
typedef struct {
    int process_id;
    harq_state_t state;
    int ndi;
    int rv;
    uint8_t *tb_data;
    size_t tb_size;
    int num_retx;
    uint8_t *soft_buffer;  // Buffer for soft combining
} harq_process_t;

/* Initialization for a HARQ process */
void harq_init_process(harq_process_t *proc, int process_id);

/* Downlink HARQ functions */

/* Handle a new downlink transmission.
   'received_ndi' is the indicator received from the PHY (from DL assignment).
   'received_rv' is the redundancy version.
   'tb_data' and 'tb_size' contain the new transport block.
   This function decides whether to treat the reception as a new transmission or as a retransmission.
*/
void harq_handle_dl_assignment(harq_process_t *proc, int received_ndi, int received_rv,
                               uint8_t *tb_data, size_t tb_size);

/* Process HARQ feedback (ACK/NACK) from the PHY for downlink.
   'ack' should be 1 if the transport block was decoded correctly, 0 otherwise.
*/
void harq_dl_process_feedback(harq_process_t *proc, int ack);

/* Uplink HARQ functions */

/* Start a new uplink transmission.
   The MAC layer provides the MAC PDU to be transmitted.
   The function stores the PDU in the HARQ buffer and instructs the PHY to transmit.
*/
void harq_ul_start_tx(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size);

/* Process uplink HARQ feedback from the PHY.
   If a NACK is received, the function should trigger a retransmission.
*/
void harq_ul_process_feedback(harq_process_t *proc, int ack);

/* Interface stubs for interaction with the physical layer and RLC */

/* Transmit a downlink transport block on the physical channel.
   This is a placeholder for the actual PHY transmission function.
*/
void phy_transmit_dl(harq_process_t *proc);

/* Instruct the PHY to combine new reception with previously stored soft data.
   'new_data' and 'new_data_size' represent the recently received TB data.
*/
void phy_combine_dl(harq_process_t *proc, uint8_t *new_data, size_t new_data_size);

/* Transmit an uplink MAC PDU.
   This function would trigger the PHY to transmit the stored uplink data.
*/
void phy_transmit_ul(harq_process_t *proc);

/* Deliver a successfully decoded MAC PDU to the RLC sublayer.
   This is a placeholder for handing the decoded data to the upper layers.
*/
void rlc_deliver_mac_pdu(uint8_t *mac_pdu, size_t pdu_size);

#endif // HARQ_H
