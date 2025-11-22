/**
 * @file set_1_challenge_1.c
 * @brief Cryptopals Set 1 Challenge 1 sample solution using hex2b64_buffer().
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hex2b64.h"

/**
 * @brief Convert the fixed challenge hex string and compare to expected Base64.
 *
 * @return EXIT_SUCCESS when the conversion matches the known answer.
 */
int
main(void)
{
	const char hex_input[] =
	    "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f"
	    "6e6f7573206d757368726f6f6d";
	const char expected_b64[] =
	    "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t\n";

	uint8_t buffer[256];
	size_t out_len = 0;
	hex2b64_status status = hex2b64_buffer((const uint8_t *) hex_input,
	    strlen(hex_input), buffer, sizeof(buffer), &out_len);
	if (status != HEX2B64_OK) {
		fprintf(stderr, "hex2b64_buffer: %s\n",
		    hex2b64_status_string(status));
		return EXIT_FAILURE;
	}
	buffer[out_len] = '\0';

	if (strcmp((const char *) buffer, expected_b64) == 0) {
		fprintf(stdout, "PASS: %s == %s\n", hex_input, expected_b64);
		return EXIT_SUCCESS;
	}

	fprintf(stdout, "FAIL: %s != %s\n", hex_input, expected_b64);
	return EXIT_FAILURE;
}
