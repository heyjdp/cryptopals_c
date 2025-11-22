/**
 * @file utils.c
 * @brief Common helper routines for hex/ASCII conversions.
 */

#include "utils.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "fixed_xor.h"
#include "score_english_hex.h"

int hex_digit_value(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

int hex_to_bytes(const char *hex, unsigned char *out, size_t out_cap) {
    if (!hex || !out) {
        return -1;
    }

    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) {
        return -1;
    }

    size_t byte_len = hex_len / 2;
    if (byte_len > out_cap) {
        return -1;
    }

    for (size_t i = 0; i < byte_len; ++i) {
        int hi = hex_digit_value((unsigned char)hex[2 * i]);
        int lo = hex_digit_value((unsigned char)hex[2 * i + 1]);
        if (hi < 0 || lo < 0) {
            return -1;
        }
        out[i] = (unsigned char)((hi << 4) | lo);
    }

    return (int)byte_len;
}

void bytes_to_hex(const unsigned char *bytes, size_t len, char *out_hex) {
    static const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[2 * i]     = digits[(bytes[i] >> 4) & 0xF];
        out_hex[2 * i + 1] = digits[bytes[i] & 0xF];
    }
    out_hex[2 * len] = '\0';
}

void hex_to_ascii(const char *hex, char *ascii_out, size_t ascii_cap) {
    if (!hex || !ascii_out || ascii_cap == 0) {
        return;
    }

    size_t hex_len = strlen(hex);
    size_t ascii_len = hex_len / 2;
    if (ascii_len >= ascii_cap) {
        ascii_len = ascii_cap - 1;
    }

    for (size_t i = 0; i < ascii_len; ++i) {
        int hi = hex_digit_value((unsigned char)hex[2 * i]);
        int lo = hex_digit_value((unsigned char)hex[2 * i + 1]);
        if (hi < 0 || lo < 0) {
            ascii_out[i] = '?';
        } else {
            ascii_out[i] = (char)((hi << 4) | lo);
        }
    }
    ascii_out[ascii_len] = '\0';
}

int brute_force_single_byte_xor(const char *hex_input,
                                unsigned char *out_plain,
                                size_t *out_len,
                                unsigned char *out_key,
                                double *out_score) {
    if (!hex_input || !out_plain || !out_len || !out_key) {
        return -1;
    }

    size_t hex_len = strlen(hex_input);
    if (hex_len == 0 || (hex_len & 1)) {
        return -1;
    }

    size_t byte_len = hex_len / 2;
    unsigned char *cipher = malloc(byte_len);
    unsigned char *candidate = malloc(byte_len);
    unsigned char *key_stream = malloc(byte_len);
    char *candidate_hex = malloc(2 * byte_len + 1);
    if (!cipher || !candidate || !key_stream || !candidate_hex) {
        free(cipher);
        free(candidate);
        free(key_stream);
        free(candidate_hex);
        return -1;
    }

    if (hex_to_bytes(hex_input, cipher, byte_len) < 0) {
        free(cipher);
        free(candidate);
        free(key_stream);
        free(candidate_hex);
        return -1;
    }

    double best_score = -1e12;
    unsigned char best_key = 0;
    memcpy(out_plain, cipher, byte_len);

    for (int key = 0; key <= 0xFF; ++key) {
        memset(key_stream, (unsigned char)key, byte_len);

        fixed_xor_status status =
            fixed_xor_buffers(cipher, key_stream, candidate, byte_len);
        if (status != FIXED_XOR_OK) {
            continue;
        }

        bytes_to_hex(candidate, byte_len, candidate_hex);
        double score = score_english_hex(candidate_hex);
        if (score > best_score) {
            best_score = score;
            best_key = (unsigned char)key;
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
    return 0;
}
