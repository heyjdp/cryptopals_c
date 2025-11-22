#ifndef FIXED_XOR_H
#define FIXED_XOR_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
    FIXED_XOR_OK = 0,
    FIXED_XOR_ERR_ARGS = -1,
    FIXED_XOR_ERR_IO = -2,
    FIXED_XOR_ERR_ODD_INPUT = -3,
    FIXED_XOR_ERR_OOM = -4
} fixed_xor_status;

fixed_xor_status fixed_xor_buffers(const unsigned char *lhs,
                                   const unsigned char *rhs,
                                   unsigned char *out,
                                   size_t len);

fixed_xor_status fixed_xor_stream(FILE *in, FILE *out);

const char *fixed_xor_status_string(fixed_xor_status status);

#endif /* FIXED_XOR_H */
