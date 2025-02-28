#ifndef PDCP_H
#define PDCP_H

#include <stddef.h>
#include <stdint.h>

// Simplified PDCP entity structure.
typedef struct {
    uint32_t tx_next;  // Next PDCP SN for transmission.
    uint32_t rx_next;  // Next PDCP SN expected.
    int header_compression_enabled;  // 1 = enabled, 0 = disabled.
    int ciphering_enabled;           // 1 = enabled, 0 = disabled.
    uint8_t cipher_key;              // Simple 8-bit key for XOR cipher.
} pdcp_entity_t;

// PDCP entity handling procedures.
void pdcp_entity_establish(pdcp_entity_t *entity);
void pdcp_entity_reestablish(pdcp_entity_t *entity);
void pdcp_entity_release(pdcp_entity_t *entity);

// PDCP data transfer procedures.
void pdcp_tx_data(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size);
void pdcp_rx_pdu(pdcp_entity_t *entity, uint8_t *pdu, size_t pdu_size);

// Interface for delivering a PDCP SDU to the upper layer.
void pdcp_deliver_sdu_to_upper(uint8_t *sdu, size_t sdu_size);

// New functions for header compression/decompression.
void pdcp_compress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size, uint8_t **output_pdu, size_t *output_size);
void pdcp_decompress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size, uint8_t **output_pdu, size_t *output_size);

// New functions for ciphering/deciphering.
void pdcp_cipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size, uint8_t **output, size_t *output_size);
void pdcp_decipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size, uint8_t **output, size_t *output_size);

// NEW: Prepare a PDCP PDU from an SDU.
// This function adds a 2-byte header carrying the PDCP SN, then applies header compression
// and ciphering if enabled. It returns the resulting PDCP PDU and its size.
uint8_t *pdcp_prepare_tx_pdu(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size, size_t *pdu_size);

// NEW: Function to obtain the global PDCP entity.
pdcp_entity_t* pdcp_get_entity(void);

#endif // PDCP_H
