/**
 * @file set_1_challenge_3.c
 * @brief Cryptopals Set 1 Challenge 3: single-byte XOR cipher.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int
main(void)
{
	const char hex_input[] =
	    "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";

	uint8_t best_plain[128];
	size_t best_len = 0;
	uint8_t best_key = 0;
	double best_score = 0.0;

	utils_status ustatus = brute_force_single_byte_xor(hex_input,
	    best_plain, sizeof(best_plain), &best_len, &best_key,
	    &best_score);
	if (ustatus != UTILS_OK) {
		fprintf(stderr, "brute force failed: %s\n",
		    utils_status_string(ustatus));
		return EXIT_FAILURE;
	}

	char best_hex[256];
	ustatus =
	    bytes_to_hex(best_plain, best_len, best_hex, sizeof(best_hex));
	if (ustatus != UTILS_OK) {
		fprintf(stderr, "bytes_to_hex failed: %s\n",
		    utils_status_string(ustatus));
		return EXIT_FAILURE;
	}

	char best_ascii[128];
	ustatus = hex_to_ascii(best_hex, best_ascii, sizeof(best_ascii));
	if (ustatus != UTILS_OK) {
		fprintf(stderr, "hex_to_ascii failed: %s\n",
		    utils_status_string(ustatus));
		return EXIT_FAILURE;
	}

	printf("Best key: 0x%02x\n", best_key);
	printf("Best plaintext (hex): %s\n", best_hex);
	printf("Best plaintext (ascii): %s\n", best_ascii);
	printf("Score: %.2f\n", best_score);

	return EXIT_SUCCESS;
}
