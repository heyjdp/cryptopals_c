/**
 * @file utils.c
 * @brief Common helper routines for hex/ASCII conversions.
 */

#include "utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_xor.h"
#include "score_english_hex.h"

const char *
utils_status_string(utils_status status)
{
	switch (status) {
	case UTILS_OK:
		return "success";
	case UTILS_ERR_ARGS:
		return "invalid arguments";
	case UTILS_ERR_INVALID_HEX:
		return "invalid hex digit";
	case UTILS_ERR_ODD_LENGTH:
		return "odd number of hex digits";
	case UTILS_ERR_BUFFER_TOO_SMALL:
		return "output buffer too small";
	case UTILS_ERR_OOM:
		return "out of memory";
	case UTILS_ERR_SCORE_FAIL:
		return "failed to score english text";
	default:
		return "unknown utils error";
	}
}

int
hex_digit_value(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	return -1;
}

utils_status
hex_to_bytes(const char *hex, uint8_t *out, size_t out_cap, size_t *out_len)
{
	if (!hex || (!out && out_cap > 0)) {
		return UTILS_ERR_ARGS;
	}

	size_t hex_len = strlen(hex);
	if (hex_len == 0) {
		if (out_len) {
			*out_len = 0;
		}
		return UTILS_OK;
	}

	if (hex_len & 1U) {
		return UTILS_ERR_ODD_LENGTH;
	}

	size_t byte_len = hex_len / 2;
	if (byte_len > out_cap) {
		return UTILS_ERR_BUFFER_TOO_SMALL;
	}

	for (size_t i = 0; i < byte_len; ++i) {
		int hi = hex_digit_value((unsigned char) hex[2 * i]);
		int lo = hex_digit_value((unsigned char) hex[2 * i + 1]);
		if (hi < 0 || lo < 0) {
			return UTILS_ERR_INVALID_HEX;
		}
		out[i] = (uint8_t) ((hi << 4) | lo);
	}

	if (out_len) {
		*out_len = byte_len;
	}
	return UTILS_OK;
}

utils_status
bytes_to_hex(const uint8_t *bytes, size_t len, char *out_hex, size_t out_cap)
{
	if (!bytes || !out_hex) {
		return UTILS_ERR_ARGS;
	}

	if ((2 * len + 1) > out_cap) {
		return UTILS_ERR_BUFFER_TOO_SMALL;
	}

	static const char digits[] = "0123456789abcdef";
	for (size_t i = 0; i < len; ++i) {
		out_hex[2 * i] = digits[(bytes[i] >> 4) & 0xF];
		out_hex[2 * i + 1] = digits[bytes[i] & 0xF];
	}
	out_hex[2 * len] = '\0';
	return UTILS_OK;
}

utils_status
hex_to_ascii(const char *hex, char *ascii_out, size_t ascii_cap)
{
	if (!hex || !ascii_out) {
		return UTILS_ERR_ARGS;
	}

	size_t hex_len = strlen(hex);
	size_t ascii_len = hex_len / 2;

	if (ascii_len + 1 > ascii_cap) {
		return UTILS_ERR_BUFFER_TOO_SMALL;
	}

	for (size_t i = 0; i < ascii_len; ++i) {
		int hi = hex_digit_value((unsigned char) hex[2 * i]);
		int lo = hex_digit_value((unsigned char) hex[2 * i + 1]);
		if (hi < 0 || lo < 0) {
			return UTILS_ERR_INVALID_HEX;
		}
		ascii_out[i] = (char) ((hi << 4) | lo);
	}

	ascii_out[ascii_len] = '\0';
	return UTILS_OK;
}

utils_status
brute_force_single_byte_xor(const char *hex_input,
    uint8_t *out_plain,
    size_t out_cap, size_t *out_len, uint8_t *out_key, double *out_score)
{
	if (!hex_input || !out_plain || !out_len || !out_key) {
		return UTILS_ERR_ARGS;
	}

	size_t hex_len = strlen(hex_input);
	if (hex_len == 0) {
		return UTILS_ERR_ARGS;
	}
	if (hex_len & 1U) {
		return UTILS_ERR_ODD_LENGTH;
	}

	size_t byte_len = hex_len / 2;
	if (byte_len > out_cap) {
		return UTILS_ERR_BUFFER_TOO_SMALL;
	}

	uint8_t *cipher = malloc(byte_len);
	uint8_t *candidate = malloc(byte_len);
	uint8_t *key_stream = malloc(byte_len);
	char *candidate_hex = malloc(2 * byte_len + 1);
	if (!cipher || !candidate || !key_stream || !candidate_hex) {
		free(cipher);
		free(candidate);
		free(key_stream);
		free(candidate_hex);
		return UTILS_ERR_OOM;
	}

	utils_status decode_status =
	    hex_to_bytes(hex_input, cipher, byte_len, NULL);
	if (decode_status != UTILS_OK) {
		free(cipher);
		free(candidate);
		free(key_stream);
		free(candidate_hex);
		return decode_status;
	}

	double best_score = -1e12;
	uint8_t best_key = 0;
	double score = 0.0;

	for (int key = 0; key <= 0xFF; ++key) {
		memset(key_stream, (uint8_t) key, byte_len);

		fixed_xor_status status =
		    fixed_xor_buffers(cipher, key_stream, candidate, byte_len);
		if (status != FIXED_XOR_OK) {
			continue;
		}

		utils_status hex_status =
		    bytes_to_hex(candidate, byte_len, candidate_hex,
		    2 * byte_len + 1);
		if (hex_status != UTILS_OK) {
			free(cipher);
			free(candidate);
			free(key_stream);
			free(candidate_hex);
			return hex_status;
		}

		score_english_hex_status score_status =
		    score_english_hex(candidate_hex, &score);
		if (score_status != SCORE_ENGLISH_HEX_OK) {
			free(cipher);
			free(candidate);
			free(key_stream);
			free(candidate_hex);
			return UTILS_ERR_SCORE_FAIL;
		}

		if (score > best_score) {
			best_score = score;
			best_key = (uint8_t) key;
			memcpy(out_plain, candidate, byte_len);
		}
	}

	*out_len = byte_len;
	*out_key = best_key;
	if (out_score) {
		*out_score = best_score;
	}

	free(cipher);
	free(candidate);
	free(key_stream);
	free(candidate_hex);
	return UTILS_OK;
}

utils_status
utils_repeat_key(const char *key, uint8_t *out, size_t buffer_len)
{
	if (!key || (!out && buffer_len > 0)) {
		return UTILS_ERR_ARGS;
	}

	size_t key_len = strlen(key);
	if (key_len == 0 && buffer_len > 0) {
		return UTILS_ERR_ARGS;
	}

	for (size_t i = 0; i < buffer_len; ++i) {
		out[i] = (uint8_t) key[i % key_len];
	}
	return UTILS_OK;
}
