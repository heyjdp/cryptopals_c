#ifndef HEX2B64_H
#define HEX2B64_H

/**
 * @file hex2b64.h
 * @brief Public interface for converting hexadecimal data to Base64.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Error states for hex-to-Base64 conversions.
 */
typedef enum
{
	HEX2B64_OK = 0,
	HEX2B64_ERR_ARGS = -1,
	HEX2B64_ERR_INVALID_HEX = -2,
	HEX2B64_ERR_ODD_DIGITS = -3,
	HEX2B64_ERR_OUTPUT_OVERFLOW = -4,
	HEX2B64_ERR_IO = -5
} hex2b64_status;

/**
 * @brief Convert hexadecimal text read from a stream into Base64.
 *
 * @param in  Input stream providing ASCII hex characters.
 * @param out Output stream that receives Base64 data.
 */
hex2b64_status hex2b64_stream(FILE * in, FILE * out);

/**
 * @brief Convert a memory buffer of hexadecimal characters into Base64.
 *
 * @param hex      Pointer to the hex buffer (may be NULL when @p hex_len is 0).
 * @param hex_len  Number of bytes in @p hex.
 * @param out      Destination buffer for Base64 characters.
 * @param out_cap  Capacity of @p out in bytes.
 * @param out_len  Optional pointer that receives the bytes produced.
 */
hex2b64_status hex2b64_buffer(const uint8_t * hex,
    size_t hex_len, uint8_t * out, size_t out_cap, size_t *out_len);

const char *hex2b64_status_string(hex2b64_status status);

#endif /* HEX2B64_H */
