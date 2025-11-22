#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hex2b64.h"

int main(void) {
    const char *hex_input =
        "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
    const char *expected_b64 = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t\n";

    FILE *in = tmpfile();
    if (!in) {
        perror("tmpfile");
        return EXIT_FAILURE;
    }

    if (fputs(hex_input, in) == EOF) {
        perror("fputs");
        fclose(in);
        return EXIT_FAILURE;
    }
    rewind(in);

    FILE *out = tmpfile();
    if (!out) {
        perror("tmpfile");
        fclose(in);
        return EXIT_FAILURE;
    }

    int rc = hex2b64_stream(in, out);
    fclose(in);
    if (rc != 0) {
        fclose(out);
        return EXIT_FAILURE;
    }

    fflush(out);
    rewind(out);

    char buffer[256];
    size_t n = fread(buffer, 1, sizeof(buffer) - 1, out);
    buffer[n] = '\0';
    fclose(out);

    if (strcmp(buffer, expected_b64) == 0) {
        fprintf(stdout, "PASS: %s == %s\n", hex_input, expected_b64);
        return EXIT_SUCCESS;
    }

    fprintf(stdout, "FAIL: %s != %s\n", hex_input, expected_b64);
    return EXIT_FAILURE;
}
