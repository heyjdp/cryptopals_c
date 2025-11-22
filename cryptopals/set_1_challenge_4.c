/**
 * @file set_1_challenge_4.c
 * @brief Detect single-byte XOR across many hex strings (Cryptopals Set 1 C4).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

typedef struct
{
	char *hex_line;
	double best_score;
	uint8_t best_key;
} line_entry;

static void
free_entries(line_entry *entries, size_t count)
{
	if (!entries)
		return;
	for (size_t i = 0; i < count; ++i) {
		free(entries[i].hex_line);
	}
	free(entries);
}

int
main(void)
{
	const char path[] = "assets/4.txt";
	FILE *file = fopen(path, "r");
	if (!file) {
		perror("Failed to open assets/4.txt");
		return EXIT_FAILURE;
	}

	size_t capacity = 64;
	size_t count = 0;
	line_entry *entries = calloc(capacity, sizeof(line_entry));
	if (!entries) {
		fclose(file);
		fprintf(stderr, "Out of memory allocating entries\n");
		return EXIT_FAILURE;
	}

	char buffer[2048];
	while (fgets(buffer, sizeof(buffer), file)) {
		size_t len = strlen(buffer);
		while (len > 0 && (buffer[len - 1] == '\n'
			|| buffer[len - 1] == '\r')) {
			buffer[--len] = '\0';
		}
		if (len == 0)
			continue;

		if (count == capacity) {
			capacity *= 2;
			line_entry *tmp =
			    realloc(entries, capacity * sizeof(line_entry));
			if (!tmp) {
				fclose(file);
				free_entries(entries, count);
				fprintf(stderr,
				    "Out of memory growing entries\n");
				return EXIT_FAILURE;
			}
			entries = tmp;
		}

		entries[count].hex_line = strdup(buffer);
		if (!entries[count].hex_line) {
			fclose(file);
			free_entries(entries, count);
			fprintf(stderr, "Out of memory copying line\n");
			return EXIT_FAILURE;
		}
		entries[count].best_score = -1e12;
		entries[count].best_key = 0;
		count++;
	}

	fclose(file);

	uint8_t candidate_plain[1024];
	size_t candidate_len = 0;

	double top_score = -1e12;
	uint8_t top_key = 0;
	size_t top_index = 0;
	size_t top_len = 0;
	uint8_t top_plain[1024];

	for (size_t idx = 0; idx < count; ++idx) {
		line_entry *entry = &entries[idx];
		double score = 0.0;
		uint8_t key = 0;

		utils_status ustatus =
		    brute_force_single_byte_xor(entry->hex_line,
		    candidate_plain, sizeof(candidate_plain),
		    &candidate_len, &key, &score);
		if (ustatus != UTILS_OK) {
			fprintf(stderr,
			    "brute force failed on line %zu: %s\n",
			    idx + 1, utils_status_string(ustatus));
			continue;
		}

		entry->best_score = score;
		entry->best_key = key;

		if (score > top_score) {
			top_score = score;
			top_key = key;
			top_index = idx;
			top_len = candidate_len;
			memcpy(top_plain, candidate_plain, candidate_len);
		}
	}

	if (top_score <= -1e11 || top_len == 0) {
		printf("No suitable candidate found.\n");
		free_entries(entries, count);
		return EXIT_SUCCESS;
	}

	char best_hex[2048];
	utils_status hex_status =
	    bytes_to_hex(top_plain, top_len, best_hex, sizeof(best_hex));
	if (hex_status != UTILS_OK) {
		fprintf(stderr, "bytes_to_hex failed: %s\n",
		    utils_status_string(hex_status));
		free_entries(entries, count);
		return EXIT_FAILURE;
	}

	char best_ascii[1024];
	hex_status = hex_to_ascii(best_hex, best_ascii, sizeof(best_ascii));
	if (hex_status != UTILS_OK) {
		fprintf(stderr, "hex_to_ascii failed: %s\n",
		    utils_status_string(hex_status));
		free_entries(entries, count);
		return EXIT_FAILURE;
	}

	printf("Best line: %zu\n", top_index + 1);
	printf("Best key: 0x%02x (%u)\n", top_key, top_key);
	printf("Score: %.2f\n", top_score);
	printf("Plaintext: %s\n", best_ascii);

	free_entries(entries, count);
	return EXIT_SUCCESS;
}
