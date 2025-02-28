#ifndef PDCP_H
#define PDCP_H

#include <stddef.h>
#include <stdint.h>

/**
 * struct pdcp_entity_t - PDCP protocol entity instance
 * @tx_next: Sequence number for next PDU transmission
 * @rx_next: Expected sequence number for next reception
 * @header_compression_enabled: Flag for header compression (1=on, 0=off)
 * @ciphering_enabled: Flag for data encryption (1=on, 0=off)
 * @cipher_key: Simple XOR encryption key (8-bit)
 *
 * Represents a PDCP entity with state information for
 * sequence numbering, header compression, and security.
 */
typedef struct {
    uint32_t tx_next;
    uint32_t rx_next;
    int header_compression_enabled;
    int ciphering_enabled;
    uint8_t cipher_key;
} pdcp_entity_t;

/* PDCP Entity Management Functions */

/**
 * pdcp_entity_establish - Create and initialize a PDCP entity
 * @entity: Pointer to PDCP entity to initialize
 *
 * Sets up a new PDCP entity with default values for sequence
 * numbers and security parameters.
 */
void pdcp_entity_establish(pdcp_entity_t *entity);

/**
 * pdcp_entity_reestablish - Reset a PDCP entity
 * @entity: Pointer to PDCP entity to reset
 *
 * Resets sequence numbers and security context while
 * maintaining configuration parameters.
 */
void pdcp_entity_reestablish(pdcp_entity_t *entity);

/**
 * pdcp_entity_release - Clean up a PDCP entity
 * @entity: Pointer to PDCP entity to release
 *
 * Releases resources and resets state of a PDCP entity
 * when it's no longer needed.
 */
void pdcp_entity_release(pdcp_entity_t *entity);

/* Data Transfer Functions */

/**
 * pdcp_tx_data - Process data for transmission
 * @entity: PDCP entity handling the transmission
 * @sdu: Data unit from upper layer
 * @sdu_size: Size of data unit in bytes
 *
 * Processes outgoing data by adding headers, applying
 * compression and encryption if enabled.
 */
void pdcp_tx_data(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size);

/**
 * pdcp_rx_pdu - Process received data
 * @entity: PDCP entity handling the reception
 * @pdu: Received protocol data unit
 * @pdu_size: Size of received data in bytes
 *
 * Processes incoming data by decrypting, decompressing,
 * and handling sequence numbers before delivery.
 */
void pdcp_rx_pdu(pdcp_entity_t *entity, uint8_t *pdu, size_t pdu_size);

/**
 * pdcp_deliver_sdu_to_upper - Forward data to upper layer
 * @sdu: Processed data unit
 * @sdu_size: Size of data unit in bytes
 *
 * Delivers processed and verified data to the upper
 * protocol layer (typically IP).
 */
void pdcp_deliver_sdu_to_upper(uint8_t *sdu, size_t sdu_size);

/* Header Compression Functions */

/**
 * pdcp_compress_header - Compress IP headers
 * @entity: PDCP entity handling compression
 * @input_pdu: Data with headers to compress
 * @input_size: Size of input data
 * @output_pdu: Resulting compressed data
 * @output_size: Size of compressed data
 *
 * Applies header compression to reduce protocol
 * overhead in the radio interface.
 */
void pdcp_compress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size,
                         uint8_t **output_pdu, size_t *output_size);

/**
 * pdcp_decompress_header - Decompress headers
 * @entity: PDCP entity handling decompression
 * @input_pdu: Compressed data
 * @input_size: Size of compressed data
 * @output_pdu: Resulting decompressed data
 * @output_size: Size of decompressed data
 *
 * Restores original headers from compressed format
 * for upper layer processing.
 */
void pdcp_decompress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size,
                           uint8_t **output_pdu, size_t *output_size);

/* Security Functions */

/**
 * pdcp_cipher - Encrypt data
 * @entity: PDCP entity with security context
 * @data: Data to encrypt
 * @data_size: Size of data to encrypt
 * @output: Resulting encrypted data
 * @output_size: Size of encrypted data
 *
 * Applies encryption to protect data confidentiality
 * using the configured cipher key.
 */
void pdcp_cipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size,
                 uint8_t **output, size_t *output_size);

/**
 * pdcp_decipher - Decrypt data
 * @entity: PDCP entity with security context
 * @data: Encrypted data
 * @data_size: Size of encrypted data
 * @output: Resulting decrypted data
 * @output_size: Size of decrypted data
 *
 * Decrypts received data using the configured
 * cipher key to restore original content.
 */
void pdcp_decipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size,
                   uint8_t **output, size_t *output_size);

/**
 * pdcp_prepare_tx_pdu - Prepare PDU for transmission
 * @entity: PDCP entity handling the transmission
 * @sdu: Input data from upper layer
 * @sdu_size: Size of input data
 * @pdu_size: Resulting PDU size
 *
 * Creates a complete PDCP PDU by:
 * 1. Adding 2-byte sequence number header
 * 2. Applying header compression if enabled
 * 3. Applying encryption if enabled
 *
 * Return: Pointer to prepared PDU or NULL on failure
 */
uint8_t *pdcp_prepare_tx_pdu(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size,
                            size_t *pdu_size);

/**
 * pdcp_get_entity - Get reference to global PDCP entity
 *
 * Return: Pointer to the global PDCP entity instance
 *
 * Provides access to the singleton PDCP entity used
 * for all PDCP layer processing.
 */
pdcp_entity_t* pdcp_get_entity(void);

#endif /* PDCP_H */
