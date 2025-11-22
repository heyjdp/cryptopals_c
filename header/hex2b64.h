#ifndef HEX2B64_H
#define HEX2B64_H

/**
 * @file hex2b64.h
 * @brief Public interface for converting hexadecimal data to Base64.
 */

#include <stddef.h>
#include <stdio.h>

/**
 * @brief Convert hexadecimal text read from a stream into Base64.
 *
 * Whitespace characters in the input stream are ignored. The output stream
 * receives the Base64 encoding plus a trailing newline. Errors are reported
 * via the return code and (for malformed hex) an explanatory message on stderr.
 *
 * @param in  Input stream providing ASCII hex characters.
 * @param out Output stream that receives Base64 data.
 * @return 0 on success, non-zero on invalid input or I/O failure.
 */
int hex2b64_stream(FILE *in, FILE *out);

/**
 * @brief Convert a memory buffer of hexadecimal characters into Base64.
 *
 * The input buffer may contain ASCII whitespace, which is skipped. The output
 * buffer receives the Base64 encoding followed by a newline. The caller must
 * supply sufficient capacity (including room for the newline). When @p out_len
 * is non-NULL it receives the number of bytes written.
 *
 * @param hex      Pointer to the hex buffer (may be NULL when @p hex_len is 0).
 * @param hex_len  Number of bytes in @p hex.
 * @param out      Destination buffer for Base64 characters.
 * @param out_cap  Capacity of @p out in bytes.
 * @param out_len  Optional pointer that receives the bytes produced.
 * @return 0 on success, non-zero on invalid hex, odd digit count, or overflow.
 */
int hex2b64_buffer(const unsigned char *hex,
                   size_t hex_len,
                   unsigned char *out,
                   size_t out_cap,
                   size_t *out_len);

#endif /* HEX2B64_H */
