#include <stdio.h>
#include <stdlib.h>

#include "fixed_xor.h"

int main(void) {
    fixed_xor_status status = fixed_xor_stream(stdin, stdout);
    if (status != FIXED_XOR_OK) {
        fprintf(stderr, "fixed_xor: %s\n", fixed_xor_status_string(status));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
