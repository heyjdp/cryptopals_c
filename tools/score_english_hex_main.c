#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "score_english_hex.h"

static double score_to_percentage(double raw) {
    if (raw < -1e8) {
        return 0.0;
    }

    double normalized = raw + 50.0;
    if (normalized < 0.0) {
        normalized = 0.0;
    }
    if (normalized > 100.0) {
        normalized = 100.0;
    }
    return normalized;
}

int main(void) {
    size_t capacity = 1024;
    size_t len = 0;
    char *hex = malloc(capacity);
    if (!hex) {
        fprintf(stderr, "score_english_hex: failed to allocate input buffer\n");
        return EXIT_FAILURE;
    }

    int ch;
    while ((ch = fgetc(stdin)) != EOF) {
        if (isspace((unsigned char)ch)) {
            continue;
        }

        if (len + 1 >= capacity) {
            size_t new_capacity = capacity * 2;
            char *tmp = realloc(hex, new_capacity);
            if (!tmp) {
                free(hex);
                fprintf(stderr, "score_english_hex: failed to grow input buffer\n");
                return EXIT_FAILURE;
            }
            hex = tmp;
            capacity = new_capacity;
        }

        hex[len++] = (char)ch;
    }
    hex[len] = '\0';

    if (len == 0) {
        free(hex);
        fprintf(stderr, "score_english_hex: no hex data provided\n");
        return EXIT_FAILURE;
    }

    double raw_score = score_english_hex(hex);
    free(hex);

    if (raw_score < -1e8) {
        fprintf(stderr, "score_english_hex: invalid hex input\n");
        return EXIT_FAILURE;
    }

    double percentage = score_to_percentage(raw_score);
    printf("%.2f\n", percentage);

    return EXIT_SUCCESS;
}
