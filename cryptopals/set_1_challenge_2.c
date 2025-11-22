/**
 * @file set_1_challenge_2.c
 * @brief Cryptopals Set 1 Challenge 2: fixed XOR test harness.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_xor.h"
#include "utils.h"

int
main(void)
{
	const char lhs_hex[] = "1c0111001f010100061a024b53535009181c";
	const char rhs_hex[] = "686974207468652062756c6c277320657965";
	const char expected_hex[] = "746865206b696420646f6e277420706c6179";

	uint8_t lhs[32];
	uint8_t rhs[32];
	uint8_t expected[32];
	uint8_t result[32];

	int lhs_len = hex_to_bytes(lhs_hex, lhs, sizeof(lhs));
	int rhs_len = hex_to_bytes(rhs_hex, rhs, sizeof(rhs));
	int expected_len =
	    hex_to_bytes(expected_hex, expected, sizeof(expected));

	if (lhs_len < 0 || rhs_len < 0 || expected_len < 0 ||
	    lhs_len != rhs_len || lhs_len != expected_len) {
		fprintf(stderr, "failed to decode hex inputs\n");
		return EXIT_FAILURE;
	}

	fixed_xor_status status =
	    fixed_xor_buffers(lhs, rhs, result, (size_t) lhs_len);
	if (status != FIXED_XOR_OK) {
		fprintf(stderr, "fixed_xor_buffers failed: %s\n",
		    fixed_xor_status_string(status));
		return EXIT_FAILURE;
	}

	if (memcmp(result, expected, (size_t) expected_len) == 0) {
		printf("PASS: %s XOR %s == %s\n", lhs_hex, rhs_hex,
		    expected_hex);
		return EXIT_SUCCESS;
	}

	printf("FAIL: result does not match expected output\n");
	return EXIT_FAILURE;
}
