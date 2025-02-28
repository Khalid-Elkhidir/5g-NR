#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <stddef.h>
#include <stdint.h>
#include "../harq/harq.h"

/**
 * mac_loopback_pdu - Simulate physical layer loopback
 * @harq: HARQ process used for uplink transmission
 * @pdu: MAC PDU to loopback
 * @pdu_size: Size of MAC PDU in bytes
 *
 * Simulates physical layer loopback by taking an uplink MAC PDU
 * and feeding it back through the downlink processing chain.
 * Uses a pre-configured global downlink RLC entity rather than
 * creating a new one for each loopback operation.
 */
void mac_loopback_pdu(harq_process_t *harq, uint8_t *pdu, size_t pdu_size);

#endif /* LOOPBACK_H */
