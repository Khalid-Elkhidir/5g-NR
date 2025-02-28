#include "harq.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * harq_init_process - Set up a new HARQ process with default values
 * @proc: Pointer to the HARQ process to initialize
 * @process_id: Unique identifier for this process
 *
 * Initializes all fields of the HARQ process structure to their default values.
 * In a real implementation, this would also allocate appropriate soft buffer size
 * based on the maximum transport block size.
 */
void harq_init_process(harq_process_t *proc, int process_id) {
    proc->process_id = process_id;
    proc->state = HARQ_IDLE;
    proc->ndi = 0;
    proc->rv = 0;
    proc->tb_data = NULL;
    proc->tb_size = 0;
    proc->num_retx = 0;
    proc->soft_buffer = NULL;
}

/**
 * harq_handle_dl_assignment - Process a new downlink transmission or retransmission
 * @proc: Target HARQ process
 * @received_ndi: New Data Indicator from physical layer
 * @received_rv: Redundancy Version for this transmission
 * @tb_data: Transport block data received
 * @tb_size: Size of the transport block
 *
 * Handles incoming downlink data by either:
 * 1. Setting up a new transmission if NDI has changed
 * 2. Combining with previous transmission if it's a retransmission
 * Manages memory for transport block and soft buffer storage.
 */
void harq_handle_dl_assignment(harq_process_t *proc, int received_ndi, int received_rv,
                             uint8_t *tb_data, size_t tb_size) {
    /* Check if this is a new transmission */
    if (proc->state == HARQ_IDLE || proc->ndi != received_ndi) {
        printf("HARQ process %d: New downlink transmission\n", proc->process_id);
        /* Clean up previous transmission data if any */
        if (proc->tb_data) {
            free(proc->tb_data);
        }
        /* Store new transmission data */
        proc->tb_data = (uint8_t *)malloc(tb_size);
        memcpy(proc->tb_data, tb_data, tb_size);
        proc->tb_size = tb_size;
        proc->ndi = received_ndi;
        proc->rv = received_rv;
        proc->num_retx = 0;
        
        /* Initialize soft buffer for potential retransmissions */
        if (proc->soft_buffer) {
            free(proc->soft_buffer);
        }
        proc->soft_buffer = (uint8_t *)malloc(tb_size);
        memcpy(proc->soft_buffer, tb_data, tb_size);
        proc->state = HARQ_WAIT_ACK;
    } else {
        /* Handle retransmission by combining with previous data */
        printf("HARQ process %d: Downlink retransmission, combining data\n", proc->process_id);
        phy_combine_dl(proc, tb_data, tb_size);
        proc->rv = received_rv;
        proc->num_retx++;
    }
}

/**
 * harq_dl_process_feedback - Handle downlink HARQ feedback
 * @proc: Target HARQ process
 * @ack: Boolean indicating successful (1) or failed (0) decoding
 *
 * Processes ACK/NACK feedback for downlink:
 * - On ACK: Delivers data to RLC and cleans up resources
 * - On NACK: Triggers retransmission through physical layer
 */
void harq_dl_process_feedback(harq_process_t *proc, int ack) {
    if (ack) {
        printf("HARQ process %d: Downlink ACK received, delivering MAC PDU to RLC\n", proc->process_id);
        /* Successful transmission - forward to RLC */
        rlc_deliver_mac_pdu(proc->tb_data, proc->tb_size);
        proc->state = HARQ_IDLE;
        /* Clean up resources */
        free(proc->tb_data);
        proc->tb_data = NULL;
        free(proc->soft_buffer);
        proc->soft_buffer = NULL;
    } else {
        printf("HARQ process %d: Downlink NACK received, scheduling retransmission\n", proc->process_id);
        /* Failed transmission - request retransmission */
        phy_transmit_dl(proc);
    }
}

/**
 * harq_ul_start_tx - Begin a new uplink transmission
 * @proc: Target HARQ process
 * @mac_pdu: MAC PDU to transmit
 * @pdu_size: Size of the MAC PDU
 *
 * Prepares and initiates a new uplink transmission:
 * 1. Stores the MAC PDU for potential retransmissions
 * 2. Initializes transmission parameters
 * 3. Triggers the physical layer transmission
 */
void harq_ul_start_tx(harq_process_t *proc, uint8_t *mac_pdu, size_t pdu_size) {
    /* Store MAC PDU for potential retransmissions */
    if (proc->tb_data) {
        free(proc->tb_data);
    }
    proc->tb_data = (uint8_t *)malloc(pdu_size);
    memcpy(proc->tb_data, mac_pdu, pdu_size);
    proc->tb_size = pdu_size;
    proc->ndi = 1;  /* Indicate new transmission */
    proc->rv = 0;
    proc->num_retx = 0;
    proc->state = HARQ_WAIT_ACK;
    
    /* Start transmission */
    phy_transmit_ul(proc);
}

/**
 * harq_ul_process_feedback - Handle uplink HARQ feedback
 * @proc: Target HARQ process
 * @ack: Boolean indicating successful (1) or failed (0) transmission
 *
 * Processes feedback for uplink transmissions:
 * - On ACK: Cleans up resources
 * - On NACK: Initiates retransmission
 */
void harq_ul_process_feedback(harq_process_t *proc, int ack) {
    if (ack) {
        printf("HARQ process %d: Uplink ACK received, transmission successful\n", proc->process_id);
        proc->state = HARQ_IDLE;
        free(proc->tb_data);
        proc->tb_data = NULL;
    } else {
        printf("HARQ process %d: Uplink NACK received, scheduling retransmission\n", proc->process_id);
        proc->num_retx++;
        phy_transmit_ul(proc);
    }
}

/**
 * phy_transmit_dl - Physical layer interface for downlink transmission
 * @proc: HARQ process containing data to transmit
 *
 * Stub function simulating physical layer transmission.
 * In a real implementation, this would interface with the actual PHY layer.
 */
void phy_transmit_dl(harq_process_t *proc) {
    printf("PHY: Retransmitting downlink TB for HARQ process %d (RV=%d, retx=%d)\n",
           proc->process_id, proc->rv, proc->num_retx);
}

/**
 * phy_combine_dl - Combine new data with stored soft bits
 * @proc: HARQ process for combining
 * @new_data: New transport block data
 * @new_data_size: Size of new transport block
 *
 * Simulates soft combining of retransmissions.
 * In practice, would implement proper Chase combining or incremental redundancy.
 */
void phy_combine_dl(harq_process_t *proc, uint8_t *new_data, size_t new_data_size) {
    /* Simple averaging for demonstration - real implementation would use proper soft combining */
    for (size_t i = 0; i < new_data_size && i < proc->tb_size; i++) {
        proc->soft_buffer[i] = (proc->soft_buffer[i] + new_data[i]) / 2;
    }
}

/**
 * phy_transmit_ul - Physical layer interface for uplink transmission
 * @proc: HARQ process containing data to transmit
 *
 * Stub function simulating physical layer transmission.
 * In a real implementation, this would interface with the actual PHY layer.
 */
void phy_transmit_ul(harq_process_t *proc) {
    printf("PHY: Transmitting uplink MAC PDU for HARQ process %d (NDI=%d, retx=%d)\n",
           proc->process_id, proc->ndi, proc->num_retx);
}

/**
 * rlc_deliver_mac_pdu - Forward decoded PDU to RLC layer
 * @mac_pdu: Successfully decoded MAC PDU
 * @pdu_size: Size of the MAC PDU
 *
 * Stub function simulating delivery to RLC layer.
 * In a real implementation, this would pass data to the actual RLC entity.
 */
void rlc_deliver_mac_pdu(uint8_t *mac_pdu, size_t pdu_size) {
    printf("RLC: Delivered MAC PDU of size %zu bytes\n", pdu_size);
}
