#include "hex2b64.h"

#include <ctype.h>
#include <stdio.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

int hex_value(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

void encode_base64_block(const unsigned char *in, size_t len, FILE *out) {
    unsigned int triple = 0;

    /* Pack bytes into 24 bits: [b0 b1 b2] -> 0x00b0b1b2 */
    triple |= (unsigned int)in[0] << 16;
    if (len > 1) triple |= (unsigned int)in[1] << 8;
    if (len > 2) triple |= (unsigned int)in[2];

    char c0 = b64_table[(triple >> 18) & 0x3F];
    char c1 = b64_table[(triple >> 12) & 0x3F];
    char c2 = (len > 1) ? b64_table[(triple >> 6) & 0x3F] : '=';
    char c3 = (len > 2) ? b64_table[triple & 0x3F]         : '=';

    fputc(c0, out);
    fputc(c1, out);
    fputc(c2, out);
    fputc(c3, out);
}

int hex2b64_stream(FILE *in, FILE *out) {
    int ch;
    int high_nibble = -1;          /* -1 means "no pending half-byte" */
    unsigned char buffer[3];       /* group bytes into blocks of 3 for Base64 */
    size_t buf_len = 0;

    while ((ch = fgetc(in)) != EOF) {
        if (isspace((unsigned char)ch)) {
            /* Ignore whitespace entirely */
            continue;
        }

        int v = hex_value(ch);
        if (v < 0) {
            fprintf(stderr, "Error: invalid hex character '%c'\n", ch);
            return 1;
        }

        if (high_nibble < 0) {
            /* First half of the byte */
            high_nibble = v;
        } else {
            /* Second half: form full byte */
            unsigned char byte = (unsigned char)((high_nibble << 4) | v);
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
        fprintf(stderr, "Error: odd number of hex digits (incomplete byte)\n");
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
