#include "harq.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Initialize a HARQ process */
void harq_init_process(harq_process_t *proc, int process_id) {
    proc->process_id = process_id;
    proc->state = HARQ_IDLE;
    proc->ndi = 0;
    proc->rv = 0;
    proc->tb_data = NULL;
    proc->tb_size = 0;
    proc->num_retx = 0;
    proc->soft_buffer = NULL; // In a real implementation, allocate a buffer of appropriate size
}

/* Downlink HARQ: Handle received DL assignment */
void harq_handle_dl_assignment(harq_process_t *proc, int received_ndi, int received_rv,
                               uint8_t *tb_data, size_t tb_size) {
    // If NDI toggled or first transmission, treat as new TB
    if (proc->state == HARQ_IDLE || proc->ndi != received_ndi) {
        // New transmission
        printf("HARQ process %d: New downlink transmission\n", proc->process_id);
        // Free previous buffer if allocated
        if (proc->tb_data) {
            free(proc->tb_data);
        }
        proc->tb_data = (uint8_t *)malloc(tb_size);
        memcpy(proc->tb_data, tb_data, tb_size);
        proc->tb_size = tb_size;
        proc->ndi = received_ndi;
        proc->rv = received_rv;
        proc->num_retx = 0;
        // Initialize soft buffer for combining (if needed)
        if (proc->soft_buffer) {
            free(proc->soft_buffer);
        }
        proc->soft_buffer = (uint8_t *)malloc(tb_size);
        memcpy(proc->soft_buffer, tb_data, tb_size);
        proc->state = HARQ_WAIT_ACK;
    } else {
        // Retransmission: combine new reception with soft buffer
        printf("HARQ process %d: Downlink retransmission, combining data\n", proc->process_id);
        // Here we simply overwrite the soft buffer with new data for illustration.
        // In practice, soft combining (e.g., Chase Combining) would be implemented.
        phy_combine_dl(proc, tb_data, tb_size);
        proc->rv = received_rv;
        proc->num_retx++;
    }
}

/* Process downlink HARQ feedback (ACK/NACK) */
void harq_dl_process_feedback(harq_process_t *proc, int ack) {
    if (ack) {
        printf("HARQ process %d: Downlink ACK received, delivering MAC PDU to RLC\n", proc->process_id);
        // Deliver the MAC PDU to the RLC sublayer
        rlc_deliver_mac_pdu(proc->tb_data, proc->tb_size);
        proc->state = HARQ_IDLE;
        // Free buffers if needed
        free(proc->tb_data);
        proc->tb_data = NULL;
        free(proc->soft_buffer);
        proc->soft_buffer = NULL;
    } else {
        printf("HARQ process %d: Downlink NACK received, scheduling retransmission\n", proc->process_id);
        // Schedule retransmission by instructing the PHY to retransmit
        phy_transmit_dl(proc);
    }
}

/* Uplink HARQ: Start a new transmission */
void harq_ul_start_tx(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size) {
    // For uplink, store the MAC PDU to be transmitted
    if (proc->tb_data) {
        free(proc->tb_data);
    }
    proc->tb_data = (uint8_t *)malloc(pdu_size);
    memcpy(proc->tb_data, mac_pdu, pdu_size);
    proc->tb_size = pdu_size;
    proc->ndi = 1;  // Set new data indicator for new transmission
    proc->rv = 0;
    proc->num_retx = 0;
    proc->state = HARQ_WAIT_ACK;
    // Instruct the PHY to transmit the uplink MAC PDU
    phy_transmit_ul(proc);
}

/* Process uplink HARQ feedback */
void harq_ul_process_feedback(harq_process_t *proc, int ack) {
    if (ack) {
        printf("HARQ process %d: Uplink ACK received, transmission successful\n", proc->process_id);
        proc->state = HARQ_IDLE;
        free(proc->tb_data);
        proc->tb_data = NULL;
    } else {
        printf("HARQ process %d: Uplink NACK received, scheduling retransmission\n", proc->process_id);
        // For a retransmission, simply re-trigger the PHY transmission with the stored data.
        proc->num_retx++;
        phy_transmit_ul(proc);
    }
}

/* Stub: PHY function to transmit downlink data (for retransmission) */
void phy_transmit_dl(harq_process_t *proc) {
    // In a real implementation, this would call the physical layer to retransmit the TB using the stored HARQ info.
    printf("PHY: Retransmitting downlink TB for HARQ process %d (RV=%d, retx=%d)\n",
           proc->process_id, proc->rv, proc->num_retx);
    // Example: send proc->soft_buffer with proc->tb_size over the PHY.
}

/* Stub: PHY function to combine new reception with soft buffer for downlink */
void phy_combine_dl(harq_process_t *proc, uint8_t *new_data, size_t new_data_size) {
    // In a real implementation, soft combining (e.g., Chase combining) would merge new_data into proc->soft_buffer.
    // For illustration, we simply average the bytes.
    for (size_t i = 0; i < new_data_size && i < proc->tb_size; i++) {
        proc->soft_buffer[i] = (proc->soft_buffer[i] + new_data[i]) / 2;
    }
}

/* Stub: PHY function to transmit uplink MAC PDU */
void phy_transmit_ul(harq_process_t *proc) {
    // In a real implementation, this would instruct the PHY to transmit the stored MAC PDU on UL-SCH.
    printf("PHY: Transmitting uplink MAC PDU for HARQ process %d (NDI=%d, retx=%d)\n",
           proc->process_id, proc->ndi, proc->num_retx);
}

/* Stub: Deliver the MAC PDU to the RLC sublayer */
void rlc_deliver_mac_pdu(uint8_t *mac_pdu, size_t pdu_size) {
    // In a real implementation, this function would pass the decoded MAC PDU to the RLC layer.
    printf("RLC: Delivered MAC PDU of size %zu bytes\n", pdu_size);
}
