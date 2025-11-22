/**
 * @file hex2b64.c
 * @brief Implementation of hex-to-Base64 conversion helpers.
 */

#include "hex2b64.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "utils.h"

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789+/";

const char *
hex2b64_status_string(hex2b64_status status)
{
	switch (status) {
	case HEX2B64_OK:
		return "success";
	case HEX2B64_ERR_ARGS:
		return "invalid arguments";
	case HEX2B64_ERR_INVALID_HEX:
		return "invalid hex digit";
	case HEX2B64_ERR_ODD_DIGITS:
		return "odd number of hex digits";
	case HEX2B64_ERR_OUTPUT_OVERFLOW:
		return "output buffer too small";
	case HEX2B64_ERR_IO:
		return "input/output failure";
	default:
		return "unknown hex2b64 error";
	}
}

static void
encode_base64_chars(const uint8_t *in, size_t len, char encoded[4])
{
	unsigned int triple = 0;

	triple |= (unsigned int) in[0] << 16;
	if (len > 1) {
		triple |= (unsigned int) in[1] << 8;
	}
	if (len > 2) {
		triple |= (unsigned int) in[2];
	}

	encoded[0] = b64_table[(triple >> 18) & 0x3F];
	encoded[1] = b64_table[(triple >> 12) & 0x3F];
	encoded[2] = (len > 1) ? b64_table[(triple >> 6) & 0x3F] : '=';
	encoded[3] = (len > 2) ? b64_table[triple & 0x3F] : '=';
}

static hex2b64_status
encode_base64_block(const uint8_t *in, size_t len, FILE *out)
{
	char encoded[4];
	encode_base64_chars(in, len, encoded);

	for (int i = 0; i < 4; ++i) {
		if (fputc(encoded[i], out) == EOF) {
			return HEX2B64_ERR_IO;
		}
	}

	return HEX2B64_OK;
}

hex2b64_status
hex2b64_stream(FILE *in, FILE *out)
{
	if (!in || !out) {
		return HEX2B64_ERR_ARGS;
	}

	int ch;
	int high_nibble = -1;
	uint8_t buffer[3];
	size_t buf_len = 0;

	while ((ch = fgetc(in)) != EOF) {
		if (isspace((unsigned char) ch)) {
			continue;
		}

		int v = hex_digit_value(ch);
		if (v < 0) {
			return HEX2B64_ERR_INVALID_HEX;
		}

		if (high_nibble < 0) {
			high_nibble = v;
		} else {
			uint8_t byte = (uint8_t) ((high_nibble << 4) | v);
			high_nibble = -1;
			buffer[buf_len++] = byte;

			if (buf_len == 3) {
				hex2b64_status status =
				    encode_base64_block(buffer, buf_len, out);
				if (status != HEX2B64_OK) {
					return status;
				}
				buf_len = 0;
			}
		}
	}

	if (ferror(in)) {
		return HEX2B64_ERR_IO;
	}

	if (high_nibble >= 0) {
		return HEX2B64_ERR_ODD_DIGITS;
	}

	if (buf_len > 0) {
		hex2b64_status status =
		    encode_base64_block(buffer, buf_len, out);
		if (status != HEX2B64_OK) {
			return status;
		}
	}

	if (fputc('\n', out) == EOF) {
		return HEX2B64_ERR_IO;
	}

	return HEX2B64_OK;
}

hex2b64_status
hex2b64_buffer(const uint8_t *hex,
    size_t hex_len, uint8_t *out, size_t out_cap, size_t *out_len)
{
	if (!out || (!hex && hex_len > 0)) {
		return HEX2B64_ERR_ARGS;
	}

	uint8_t buffer[3];
	size_t buf_len = 0;
	int high_nibble = -1;
	size_t produced = 0;

	for (size_t i = 0; i < hex_len; ++i) {
		uint8_t ch = hex[i];
		if (isspace((unsigned char) ch)) {
			continue;
		}

		int v = hex_digit_value(ch);
		if (v < 0) {
			return HEX2B64_ERR_INVALID_HEX;
		}

		if (high_nibble < 0) {
			high_nibble = v;
		} else {
			uint8_t byte = (uint8_t) ((high_nibble << 4) | v);
			high_nibble = -1;
			buffer[buf_len++] = byte;

			if (buf_len == 3) {
				if (produced + 4 > out_cap) {
					return HEX2B64_ERR_OUTPUT_OVERFLOW;
				}
				char encoded[4];
				encode_base64_chars(buffer, buf_len, encoded);
				out[produced++] = (uint8_t) encoded[0];
				out[produced++] = (uint8_t) encoded[1];
				out[produced++] = (uint8_t) encoded[2];
				out[produced++] = (uint8_t) encoded[3];
				buf_len = 0;
			}
		}
	}

	if (high_nibble >= 0) {
		return HEX2B64_ERR_ODD_DIGITS;
	}

	if (buf_len > 0) {
		if (produced + 4 > out_cap) {
			return HEX2B64_ERR_OUTPUT_OVERFLOW;
		}
		char encoded[4];
		encode_base64_chars(buffer, buf_len, encoded);
		out[produced++] = (uint8_t) encoded[0];
		out[produced++] = (uint8_t) encoded[1];
		out[produced++] = (uint8_t) encoded[2];
		out[produced++] = (uint8_t) encoded[3];
	}

	if (produced + 1 > out_cap) {
		return HEX2B64_ERR_OUTPUT_OVERFLOW;
	}
	out[produced++] = '\n';

	if (out_len) {
		*out_len = produced;
	}

	return HEX2B64_OK;
}
