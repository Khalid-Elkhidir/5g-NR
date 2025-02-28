#include "pdcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global PDCP entity instance.
static pdcp_entity_t global_pdcp_entity;

void pdcp_entity_establish(pdcp_entity_t *entity) {
    if (!entity) return;
    entity->tx_next = 0;
    entity->rx_next = 0;
    entity->header_compression_enabled = 1;  // Enabled by default.
    entity->ciphering_enabled = 1;           // Enabled by default.
    entity->cipher_key = 0x5A;               // Example key.
    printf("PDCP: Entity established. TX_NEXT and RX_NEXT set to 0. Compression and ciphering enabled.\n");
}

void pdcp_entity_reestablish(pdcp_entity_t *entity) {
    if (!entity) return;
    entity->tx_next = 0;
    entity->rx_next = 0;
    printf("PDCP: Entity re-established. TX_NEXT and RX_NEXT reset to 0.\n");
}

void pdcp_entity_release(pdcp_entity_t *entity) {
    if (!entity) return;
    printf("PDCP: Entity released.\n");
}

void pdcp_tx_data(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size) {
    // For backward compatibility: prepare the PDCP PDU and then "send" it.
    size_t pdu_size = 0;
    uint8_t *pdu = pdcp_prepare_tx_pdu(entity, sdu, sdu_size, &pdu_size);
    if (!pdu) return;
    printf("PDCP: Sending PDCP Data PDU to lower layer (simulated).\n");
    // In a full implementation, pdu would be forwarded to the RLC layer.
    free(pdu);
}

uint8_t *pdcp_prepare_tx_pdu(pdcp_entity_t *entity, uint8_t *sdu, size_t sdu_size, size_t *pdu_size) {
    if (!entity || !sdu) return NULL;
    // Create a 2-byte header carrying the PDCP SN.
    uint16_t sn = (uint16_t)(entity->tx_next & 0x0FFF); // Use lower 12 bits.
    uint8_t header[2];
    header[0] = (sn >> 4) & 0xFF;
    header[1] = ((sn & 0x0F) << 4);  // Lower nibble padded with zeros.
    size_t raw_size = 2 + sdu_size;
    uint8_t *raw_pdu = malloc(raw_size);
    if (!raw_pdu) return NULL;
    memcpy(raw_pdu, header, 2);
    memcpy(raw_pdu + 2, sdu, sdu_size);
    entity->tx_next++;

    // Apply header compression if enabled.
    uint8_t *comp_pdu = raw_pdu;
    size_t comp_size = raw_size;
    if (entity->header_compression_enabled) {
        pdcp_compress_header(entity, raw_pdu, raw_size, &comp_pdu, &comp_size);
        free(raw_pdu);
        printf("PDCP: Header compressed. Size reduced to %zu bytes.\n", comp_size);
    }
    
    // Apply ciphering if enabled.
    uint8_t *cipher_pdu = comp_pdu;
    size_t cipher_size = comp_size;
    if (entity->ciphering_enabled) {
        pdcp_cipher(entity, comp_pdu, comp_size, &cipher_pdu, &cipher_size);
        if (comp_pdu != cipher_pdu)
            free(comp_pdu);
        printf("PDCP: PDU ciphered. Size is now %zu bytes.\n", cipher_size);
    }
    
    *pdu_size = cipher_size;
    return cipher_pdu;
}

void pdcp_rx_pdu(pdcp_entity_t *entity, uint8_t *pdu, size_t pdu_size) {
    if (!entity || !pdu || pdu_size < 1) {
        printf("PDCP: Invalid PDU received\n");
        return;
    }
    
    uint8_t *deciphered = pdu;
    size_t decipher_size = pdu_size;
    if (entity->ciphering_enabled) {
        pdcp_decipher(entity, pdu, pdu_size, &deciphered, &decipher_size);
    }
    
    uint8_t *decomp = deciphered;
    size_t decomp_size = decipher_size;
    if (entity->header_compression_enabled && deciphered[0] == 0xAA) {
        pdcp_decompress_header(entity, deciphered, decipher_size, &decomp, &decomp_size);
    }
    
    if (decomp_size < 2) {
        printf("PDCP: Invalid decompressed PDU\n");
        if (decomp != deciphered) free(decomp);
        if (deciphered != pdu) free(deciphered);
        return;
    }
    
    uint16_t sn = ((uint16_t)decomp[0] << 4) | (decomp[1] >> 4);
    printf("PDCP: Received PDU with SN = %u\n", sn);
    
    size_t sdu_size = decomp_size - 2;
    uint8_t *sdu = malloc(sdu_size);
    if (!sdu) {
        if (decomp != deciphered) free(decomp);
        if (deciphered != pdu) free(deciphered);
        return;
    }
    memcpy(sdu, decomp + 2, sdu_size);
    
    entity->rx_next = sn + 1;
    
    pdcp_deliver_sdu_to_upper(sdu, sdu_size);
    free(sdu);
    if (decomp != deciphered) free(decomp);
    if (deciphered != pdu) free(deciphered);
}

void pdcp_deliver_sdu_to_upper(uint8_t *sdu, size_t sdu_size) {
    if (!sdu) return;
    printf("PDCP: Delivered PDCP SDU to upper layer: %s\n", sdu);
}

// --- Header Compression/Decompression (Simulated ROHC) ---
void pdcp_compress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size, uint8_t **output_pdu, size_t *output_size) {
    *output_size = input_size + 1;
    *output_pdu = malloc(*output_size);
    if (!*output_pdu) return;
    (*output_pdu)[0] = 0xAA; // Compression marker.
    memcpy(*output_pdu + 1, input_pdu, input_size);
}

void pdcp_decompress_header(pdcp_entity_t *entity, uint8_t *input_pdu, size_t input_size, uint8_t **output_pdu, size_t *output_size) {
    if (input_size < 1) return;
    if (input_pdu[0] == 0xAA) {
        *output_size = input_size - 1;
        *output_pdu = malloc(*output_size);
        if (!*output_pdu) return;
        memcpy(*output_pdu, input_pdu + 1, *output_size);
    } else {
        *output_size = input_size;
        *output_pdu = malloc(*output_size);
        if (!*output_pdu) return;
        memcpy(*output_pdu, input_pdu, *output_size);
    }
}

// --- Ciphering/Deciphering (Simulated using XOR) ---
void pdcp_cipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size, uint8_t **output, size_t *output_size) {
    *output_size = data_size;
    *output = malloc(data_size);
    if (!*output) return;
    for (size_t i = 0; i < data_size; i++) {
        (*output)[i] = data[i] ^ entity->cipher_key;
    }
}

void pdcp_decipher(pdcp_entity_t *entity, uint8_t *data, size_t data_size, uint8_t **output, size_t *output_size) {
    // XOR deciphering is identical to ciphering.
    pdcp_cipher(entity, data, data_size, output, output_size);
}

// --- Return global PDCP entity ---
pdcp_entity_t* pdcp_get_entity(void) {
    static int is_initialized = 0;
    if (!is_initialized) {
        pdcp_entity_establish(&global_pdcp_entity);
        is_initialized = 1;
    }
    return &global_pdcp_entity;
}
