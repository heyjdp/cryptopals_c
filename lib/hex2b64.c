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

/**
 * @brief Produce four Base64 characters from up to three bytes.
 *
 * @param in       Input bytes (padded implicitly with zeros).
 * @param len      Number of valid bytes (1-3).
 * @param encoded  Destination for the four encoded characters.
 */
static void
encode_base64_chars(const uint8_t *in, size_t len, char encoded[4])
{
	unsigned int triple = 0;

	/* Pack bytes into 24 bits: [b0 b1 b2] -> 0x00b0b1b2 */
	triple |= (unsigned int) in[0] << 16;
	if (len > 1)
		triple |= (unsigned int) in[1] << 8;
	if (len > 2)
		triple |= (unsigned int) in[2];

	encoded[0] = b64_table[(triple >> 18) & 0x3F];
	encoded[1] = b64_table[(triple >> 12) & 0x3F];
	encoded[2] = (len > 1) ? b64_table[(triple >> 6) & 0x3F] : '=';
	encoded[3] = (len > 2) ? b64_table[triple & 0x3F] : '=';
}

/**
 * @brief Write a Base64 block derived from up to three input bytes.
 *
 * @param in  Source bytes.
 * @param len Number of source bytes.
 * @param out Output stream that receives four Base64 characters.
 */
static void
encode_base64_block(const uint8_t *in, size_t len, FILE *out)
{
	char encoded[4];
	encode_base64_chars(in, len, encoded);

	fputc(encoded[0], out);
	fputc(encoded[1], out);
	fputc(encoded[2], out);
	fputc(encoded[3], out);
}

/** @brief Implementation of hex2b64_stream(). */
int
hex2b64_stream(FILE *in, FILE *out)
{
	int ch;
	int high_nibble = -1;	/* -1 means "no pending half-byte" */
	uint8_t buffer[3];	/* group bytes into blocks of 3 for Base64 */
	size_t buf_len = 0;

	while ((ch = fgetc(in)) != EOF) {
		if (isspace((uint8_t) ch)) {
			/* Ignore whitespace entirely */
			continue;
		}

		int v = hex_digit_value(ch);
		if (v < 0) {
			fprintf(stderr, "Error: invalid hex character '%c'\n",
			    ch);
			return 1;
		}

		if (high_nibble < 0) {
			/* First half of the byte */
			high_nibble = v;
		} else {
			/* Second half: form full byte */
			uint8_t byte = (uint8_t) ((high_nibble << 4) | v);
			high_nibble = -1;

			buffer[buf_len++] = byte;

			if (buf_len == 3) {
				encode_base64_block(buffer, buf_len, out);
				buf_len = 0;
			}
		}
	}

	if (high_nibble >= 0) {
		/* Odd number of hex digits: cannot form a full byte */
		fprintf(stderr,
		    "Error: odd number of hex digits (incomplete byte)\n");
		return 1;
	}

	/* Flush remaining bytes (if 1 or 2 bytes) with padding */
	if (buf_len > 0) {
		encode_base64_block(buffer, buf_len, out);
	}

	/* Optional trailing newline */
	fputc('\n', out);

	return 0;
}

/** @brief Implementation of hex2b64_buffer(). */
int
hex2b64_buffer(const uint8_t *hex,
    size_t hex_len, uint8_t *out, size_t out_cap, size_t *out_len)
{
	if (!out || (hex_len > 0 && !hex)) {
		return 1;
	}

	uint8_t buffer[3];
	size_t buf_len = 0;
	int high_nibble = -1;
	size_t produced = 0;

	for (size_t i = 0; i < hex_len; ++i) {
		uint8_t ch = hex[i];
		if (isspace(ch)) {
			continue;
		}

		int v = hex_digit_value(ch);
		if (v < 0) {
			return 1;
		}

		if (high_nibble < 0) {
			high_nibble = v;
		} else {
			uint8_t byte = (uint8_t) ((high_nibble << 4) | v);
			high_nibble = -1;
			buffer[buf_len++] = byte;

			if (buf_len == 3) {
				if (produced + 4 > out_cap) {
					return 1;
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
		return 1;
	}

	if (buf_len > 0) {
		if (produced + 4 > out_cap) {
			return 1;
		}
		char encoded[4];
		encode_base64_chars(buffer, buf_len, encoded);
		out[produced++] = (uint8_t) encoded[0];
		out[produced++] = (uint8_t) encoded[1];
		out[produced++] = (uint8_t) encoded[2];
		out[produced++] = (uint8_t) encoded[3];
	}

	if (produced + 1 > out_cap) {
		return 1;
	}
	out[produced++] = '\n';

	if (out_len) {
		*out_len = produced;
	}

	return 0;
}
