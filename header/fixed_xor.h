#ifndef FIXED_XOR_H
#define FIXED_XOR_H

/**
 * @file fixed_xor.h
 * @brief Public interface for XORing two equally sized buffers.
 */

#include <stddef.h>
#include <stdio.h>

/**
 * @brief Status codes describing the outcome of fixed XOR operations.
 */
typedef enum {
    FIXED_XOR_OK = 0,             /**< Operation completed successfully. */
    FIXED_XOR_ERR_ARGS = -1,      /**< Invalid arguments were supplied. */
    FIXED_XOR_ERR_IO = -2,        /**< I/O failure while reading or writing. */
    FIXED_XOR_ERR_ODD_INPUT = -3, /**< Stream input contained an odd byte count. */
    FIXED_XOR_ERR_OOM = -4        /**< Memory allocation failed. */
} fixed_xor_status;

/**
 * @brief XOR two buffers of equal length into an output buffer.
 *
 * @param lhs Pointer to the left-hand buffer.
 * @param rhs Pointer to the right-hand buffer.
 * @param out Destination buffer that receives lhs ^ rhs.
 * @param len Number of bytes to process.
 * @return FIXED_XOR_OK on success or an error status on failure.
 */
fixed_xor_status fixed_xor_buffers(const unsigned char *lhs,
                                   const unsigned char *rhs,
                                   unsigned char *out,
                                   size_t len);

/**
 * @brief XOR two half-length buffers read sequentially from a stream.
 *
 * The input stream is expected to contain two equally sized buffers back to
 * back. The result is written to @p out.
 *
 * @param in  Stream containing the concatenated buffers.
 * @param out Stream that receives XOR output.
 * @return FIXED_XOR_OK on success or an error status on failure.
 */
fixed_xor_status fixed_xor_stream(FILE *in, FILE *out);

/**
 * @brief Convert a fixed_xor_status value into a human-readable string.
 *
 * @param status Status code to describe.
 * @return Pointer to a static string literal.
 */
const char *fixed_xor_status_string(fixed_xor_status status);

#endif /* FIXED_XOR_H */
