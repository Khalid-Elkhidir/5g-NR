#ifndef RLC_H
#define RLC_H

#include <stddef.h>
#include <stdint.h>
#include "../mac/mac.h"   // RLC uses MAC interface for data transmission

typedef enum {
    RLC_MODE_TM,
    RLC_MODE_UM,
    RLC_MODE_AM
} rlc_mode_t;

/* RLC entity structure.
   For UM mode, we add a reassembly buffer to accumulate segments.
*/
typedef struct {
    rlc_mode_t mode;
    uint8_t tx_next;  // Next sequence number for transmission (for UM/AM)
    uint8_t rx_next;  // Next expected sequence number (for UM/AM)
    uint8_t *reassembly_buffer;
    size_t reassembly_size;
    uint8_t reassembly_sn;  // Sequence number of the SDU currently under reassembly
} rlc_entity_t;

/* RLC entity handling functions */
void rlc_entity_establish(rlc_entity_t *entity, rlc_mode_t mode);
void rlc_entity_reestablish(rlc_entity_t *entity);
void rlc_entity_release(rlc_entity_t *entity);

/* TM mode functions */
void rlc_tm_tx_data(rlc_entity_t *entity, uint8_t *sdu, size_t sdu_size);
void rlc_tm_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size);

/* UM mode functions */
#define RLC_UM_SEGMENT_SIZE 20
void rlc_um_tx_data(rlc_entity_t *entity, uint8_t *sdu, size_t sdu_size);
void rlc_um_rx_data(rlc_entity_t *entity, uint8_t *pdu, size_t pdu_size);

#endif // RLC_H
