/**
 * @file fixed_xor_main.c
 * @brief Command-line wrapper for XORing two equal-length buffers.
 */

#include <stdio.h>
#include <stdlib.h>

#include "fixed_xor.h"

/**
 * @brief Entry point that streams stdin through fixed_xor_stream().
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
int main(void) {
    fixed_xor_status status = fixed_xor_stream(stdin, stdout);
    if (status != FIXED_XOR_OK) {
        fprintf(stderr, "fixed_xor: %s\n", fixed_xor_status_string(status));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
