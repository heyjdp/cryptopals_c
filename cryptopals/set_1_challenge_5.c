/**
 * @file set_1_challenge_5.c
 * @brief Cryptopals Set 1 Challenge 5: repeating-key XOR.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_xor.h"
#include "utils.h"

static int
read_file_into_buffer(const char *path, uint8_t **out_buf, size_t *out_len)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		perror("fopen");
		return -1;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		perror("fseek");
		fclose(file);
		return -1;
	}

	long size = ftell(file);
	if (size < 0) {
		perror("ftell");
		fclose(file);
		return -1;
	}
	rewind(file);

	uint8_t *buffer = malloc((size_t) size);
	if (!buffer) {
		fclose(file);
		return -1;
	}

	size_t read_len = fread(buffer, 1, (size_t) size, file);
	fclose(file);

	if (read_len != (size_t) size) {
		free(buffer);
		fprintf(stderr, "Failed to read entire file\n");
		return -1;
	}

	*out_buf = buffer;
	*out_len = read_len;
	return 0;
}

int
main(void)
{
	const char path[] = "assets/5.txt";
	uint8_t *plaintext = NULL;
	size_t plaintext_len = 0;

	if (read_file_into_buffer(path, &plaintext, &plaintext_len) != 0) {
		return EXIT_FAILURE;
	}

	if (plaintext_len > 0 && plaintext[plaintext_len - 1] == '\n') {
		plaintext_len--;
	}
	if (plaintext_len > 0 && plaintext[plaintext_len - 1] == '\r') {
		plaintext_len--;
	}

	const char key[] = "ICE";
	uint8_t *key_stream = malloc(plaintext_len);
	uint8_t *cipher = malloc(plaintext_len);
	char *cipher_hex = malloc(plaintext_len * 2 + 1);
	if (!key_stream || !cipher || !cipher_hex) {
		fprintf(stderr, "Allocation failure\n");
		free(plaintext);
		free(key_stream);
		free(cipher);
		free(cipher_hex);
		return EXIT_FAILURE;
	}

	utils_status ustatus =
	    utils_repeat_key(key, key_stream, plaintext_len);
	if (ustatus != UTILS_OK) {
		fprintf(stderr, "utils_repeat_key failed: %s\n",
		    utils_status_string(ustatus));
		free(plaintext);
		free(key_stream);
		free(cipher);
		free(cipher_hex);
		return EXIT_FAILURE;
	}

	fixed_xor_status fx_status =
	    fixed_xor_buffers(plaintext, key_stream, cipher, plaintext_len);
	if (fx_status != FIXED_XOR_OK) {
		fprintf(stderr, "fixed_xor_buffers failed: %s\n",
		    fixed_xor_status_string(fx_status));
		free(plaintext);
		free(key_stream);
		free(cipher);
		free(cipher_hex);
		return EXIT_FAILURE;
	}

	ustatus = bytes_to_hex(cipher,
	    plaintext_len, cipher_hex, plaintext_len * 2 + 1);
	if (ustatus != UTILS_OK) {
		fprintf(stderr, "bytes_to_hex failed: %s\n",
		    utils_status_string(ustatus));
		free(plaintext);
		free(key_stream);
		free(cipher);
		free(cipher_hex);
		return EXIT_FAILURE;
	}

	const char expected_hex[] =
	    "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324"
	    "272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b2028316528"
	    "6326302e27282f";

	int result = strcmp(cipher_hex, expected_hex);
	if (result == 0) {
		printf("PASS: repeating-key XOR matches expected output\n");
	} else {
		printf("FAIL: expected\n%s\nbut got\n%s\n", expected_hex,
		    cipher_hex);
		free(plaintext);
		free(key_stream);
		free(cipher);
		free(cipher_hex);
		return EXIT_FAILURE;
	}

	free(plaintext);
	free(key_stream);
	free(cipher);
	free(cipher_hex);
	return EXIT_SUCCESS;
}
