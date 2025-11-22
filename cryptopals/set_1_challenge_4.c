/**
 * @file set_1_challenge_4.c
 * @brief Detect single-byte XOR across many hex strings (Cryptopals Set 1 C4).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_xor.h"
#include "score_english_hex.h"
#include "utils.h"

typedef struct {
    char *hex_line;
    double best_score;
    unsigned char best_key;
} line_entry;

static void free_entries(line_entry *entries, size_t count) {
    if (!entries) return;
    for (size_t i = 0; i < count; ++i) {
        free(entries[i].hex_line);
    }
    free(entries);
}

int main(void) {
    const char *path = "assets/4.txt";
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
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[--len] = '\0';
        }
        if (len == 0) continue;

        if (count == capacity) {
            capacity *= 2;
            line_entry *tmp = realloc(entries, capacity * sizeof(line_entry));
            if (!tmp) {
                fclose(file);
                free_entries(entries, count);
                fprintf(stderr, "Out of memory growing entries\n");
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

    unsigned char best_plain[1024];
    size_t best_len = 0;

    for (size_t idx = 0; idx < count; ++idx) {
        line_entry *entry = &entries[idx];

        double score = 0.0;
        unsigned char key = 0;
        if (brute_force_single_byte_xor(entry->hex_line,
                                        best_plain,
                                        &best_len,
                                        &key,
                                        &score) != 0) {
            continue;
        }

        entry->best_score = score;
        entry->best_key = key;
    }

    double top_score = -1e12;
    unsigned char top_key = 0;
    size_t top_index = 0;
    size_t top_len = 0;
    unsigned char top_plain[1024];

    for (size_t i = 0; i < count; ++i) {
        if (entries[i].best_score > top_score) {
            top_score = entries[i].best_score;
            top_key = entries[i].best_key;
            top_index = i;
            top_len = 0;

            double tmp_score;
            unsigned char tmp_key;
            if (brute_force_single_byte_xor(entries[i].hex_line,
                                            top_plain,
                                            &top_len,
                                            &tmp_key,
                                            &tmp_score) != 0) {
                top_len = 0;
            }
        }
    }

    if (top_score <= -1e11) {
        printf("No suitable candidate found.\n");
    } else {
        char best_hex[2048];
        char best_ascii[1024];
        if (top_len > 0) {
            bytes_to_hex(top_plain, top_len, best_hex);
            hex_to_ascii(best_hex, best_ascii, sizeof(best_ascii));
        } else {
            best_ascii[0] = '\0';
        }

        printf("Best line: %zu\n", top_index + 1);
        printf("Best key: 0x%02x (%u)\n", top_key, top_key);
        printf("Score: %.2f\n", top_score);
        if (top_len > 0) {
            printf("Plaintext: %s\n", best_ascii);
        }
    }

    free_entries(entries, count);
    return EXIT_SUCCESS;
}
