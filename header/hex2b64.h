#ifndef HEX2B64_H
#define HEX2B64_H

#include <stdio.h>
#include <stddef.h>

/* Convert a single hex digit to 0–15, or -1 on error. */
int hex_value(int c);

/* Encode up to 3 bytes to Base64, writing exactly 4 chars to out FILE. */
void encode_base64_block(const unsigned char *in, size_t len, FILE *out);

/* High-level function: read hex from in, write Base64 to out.
   Returns 0 on success, non-zero on error. */
int hex2b64_stream(FILE *in, FILE *out);

#endif /* HEX2B64_H */
