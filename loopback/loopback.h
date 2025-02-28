#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <stddef.h>
#include <stdint.h>
#include "../harq/harq.h"

/* 
 * Function: mac_loopback_pdu
 * --------------------------
 * Simulates the PHY layer loopback of a MAC PDU.
 *
 * Parameters:
 *   harq - pointer to the HARQ process used in the uplink transmission.
 *   pdu  - pointer to the MAC PDU buffer.
 *   pdu_size - size of the MAC PDU.
 *
 * This function does not create a new RLC entity; instead, it uses a global
 * downlink RLC entity (set externally) to trigger the MAC downlink processing.
 */
void mac_loopback_pdu(harq_process_t *harq, uint8_t *pdu, size_t pdu_size);

#endif // LOOPBACK_H
