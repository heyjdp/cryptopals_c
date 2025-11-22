/**
 * @file fixed_xor.c
 * @brief Implementation of fixed-length buffer XOR helpers.
 */

#include "fixed_xor.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FIXED_XOR_CHUNK 4096

static int fixed_xor_force_oom = 0;

void
fixed_xor_set_allocation_failure(int enable)
{
	fixed_xor_force_oom = enable;
}

/** @brief Implementation of fixed_xor_buffers(). */
fixed_xor_status
fixed_xor_buffers(const uint8_t *lhs,
    const uint8_t *rhs, uint8_t *out, size_t len)
{
	if (!lhs || !rhs || !out) {
		errno = EINVAL;
		return FIXED_XOR_ERR_ARGS;
	}

	for (size_t i = 0; i < len; ++i) {
		out[i] = lhs[i] ^ rhs[i];
	}

	return FIXED_XOR_OK;
}

/**
 * @brief Ensure that @p buffer has capacity for @p required bytes.
 *
 * @param buffer   Pointer to heap storage pointer to grow.
 * @param capacity Current capacity in bytes (updated on success).
 * @param required Target capacity required by the caller.
 * @return FIXED_XOR_OK on success or FIXED_XOR_ERR_OOM on allocation failure.
 */
static fixed_xor_status
fixed_xor_grow(uint8_t **buffer, size_t *capacity, size_t required)
{
	if (fixed_xor_force_oom) {
		return FIXED_XOR_ERR_OOM;
	}

	size_t new_capacity = *capacity;
	while (required > new_capacity) {
		if (new_capacity > (SIZE_MAX / 2)) {
			return FIXED_XOR_ERR_OOM;
		}
		new_capacity *= 2;
	}

	if (new_capacity == *capacity) {
		return FIXED_XOR_OK;
	}

	uint8_t *new_data = realloc(*buffer, new_capacity);
	if (!new_data) {
		return FIXED_XOR_ERR_OOM;
	}

	*buffer = new_data;
	*capacity = new_capacity;
	return FIXED_XOR_OK;
}

/** @brief Implementation of fixed_xor_stream(). */
fixed_xor_status
fixed_xor_stream(FILE *in, FILE *out)
{
	if (!in || !out) {
		errno = EINVAL;
		return FIXED_XOR_ERR_ARGS;
	}
	if (fixed_xor_force_oom) {
		return FIXED_XOR_ERR_OOM;
	}

	size_t capacity = FIXED_XOR_CHUNK;
	uint8_t *data = malloc(capacity);
	if (!data) {
		return FIXED_XOR_ERR_OOM;
	}

	size_t length = 0;
	uint8_t chunk[FIXED_XOR_CHUNK];
	size_t nread;

	while ((nread = fread(chunk, 1, sizeof(chunk), in)) > 0) {
		if (length + nread > capacity) {
			fixed_xor_status grow_status =
			    fixed_xor_grow(&data, &capacity, length + nread);
			if (grow_status != FIXED_XOR_OK) {
				free(data);
				return grow_status;
			}
		}
		memcpy(data + length, chunk, nread);
		length += nread;
	}

	if (ferror(in)) {
		free(data);
		return FIXED_XOR_ERR_IO;
	}

	if (length % 2 != 0) {
		free(data);
		return FIXED_XOR_ERR_ODD_INPUT;
	}

	size_t half = length / 2;
	fixed_xor_status status =
	    fixed_xor_buffers(data, data + half, data, half);
	if (status != FIXED_XOR_OK) {
		free(data);
		return status;
	}

	size_t nwritten = fwrite(data, 1, half, out);
	if (nwritten != half) {
		free(data);
		return FIXED_XOR_ERR_IO;
	}

	free(data);
	return FIXED_XOR_OK;
}

/** @brief Implementation of fixed_xor_status_string(). */
const char *
fixed_xor_status_string(fixed_xor_status status)
{
	switch (status) {
	case FIXED_XOR_OK:
		return "success";
	case FIXED_XOR_ERR_ARGS:
		return "invalid arguments";
	case FIXED_XOR_ERR_IO:
		return "I/O failure";
	case FIXED_XOR_ERR_ODD_INPUT:
		return "input length must be even (two equal buffers)";
	case FIXED_XOR_ERR_OOM:
		return "out of memory";
	default:
		return "unknown error";
	}
}
