#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Shared helpers for hex and single-byte XOR operations.
 */

#include <stddef.h>
#include <stdint.h>

typedef enum
{
	UTILS_OK = 0,
	UTILS_ERR_ARGS = -1,
	UTILS_ERR_INVALID_HEX = -2,
	UTILS_ERR_ODD_LENGTH = -3,
	UTILS_ERR_BUFFER_TOO_SMALL = -4,
	UTILS_ERR_OOM = -5,
	UTILS_ERR_SCORE_FAIL = -6
} utils_status;

int hex_digit_value(int c);

utils_status hex_to_bytes(const char *hex,
    uint8_t * out, size_t out_cap, size_t *out_len);

utils_status bytes_to_hex(const uint8_t * bytes,
    size_t len, char *out_hex, size_t out_cap);

utils_status hex_to_ascii(const char *hex, char *ascii_out, size_t ascii_cap);

utils_status brute_force_single_byte_xor(const char *hex_input,
    uint8_t * out_plain,
    size_t out_cap, size_t *out_len, uint8_t * out_key, double *out_score);

utils_status utils_repeat_key(const char *key,
    uint8_t * out, size_t buffer_len);

const char *utils_status_string(utils_status status);

#endif /* UTILS_H */
