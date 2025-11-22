#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/**
 * @brief Convert a single hexadecimal digit to its integer value.
 *
 * @param c Character to convert.
 * @return 0-15 for valid hex digits, or -1 on invalid input.
 */
int hex_digit_value(int c);

/**
 * @brief Decode a hex string into bytes.
 *
 * @param hex     Null-terminated hex string.
 * @param out     Destination buffer for decoded bytes.
 * @param out_cap Capacity of @p out.
 * @return Number of bytes written on success, or -1 on error.
 */
int hex_to_bytes(const char *hex, unsigned char *out, size_t out_cap);

/**
 * @brief Encode bytes into a lowercase hex string.
 *
 * @param bytes   Source buffer.
 * @param len     Number of bytes to encode.
 * @param out_hex Destination buffer (must be at least 2*len+1).
 */
void bytes_to_hex(const unsigned char *bytes, size_t len, char *out_hex);

/**
 * @brief Convert a hex string into ASCII characters (best-effort).
 *
 * @param hex        Source hex string.
 * @param ascii_out  Destination buffer for ASCII text.
 * @param ascii_cap  Capacity of @p ascii_out.
 */
void hex_to_ascii(const char *hex, char *ascii_out, size_t ascii_cap);

/**
 * @brief Brute-force single-byte XOR keys for a hex-encoded string.
 *
 * @param hex_input    Null-terminated hex ciphertext.
 * @param out_plain    Destination buffer for best plaintext bytes.
 * @param out_len      Receives length of plaintext on success.
 * @param out_key      Receives best key byte on success.
 * @param out_score    Receives best score (may be NULL).
 * @return 0 on success, non-zero on error (invalid hex or allocation failure).
 */
int brute_force_single_byte_xor(const char *hex_input,
                                unsigned char *out_plain,
                                size_t *out_len,
                                unsigned char *out_key,
                                double *out_score);

#endif /* UTILS_H */
