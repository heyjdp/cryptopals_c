/**
 * @file hex2b64_main.c
 * @brief Command-line tool that converts hex input to Base64.
 */

#include <stdio.h>
#include <stdlib.h>

#include "hex2b64.h"

int
main(void)
{
	hex2b64_status status = hex2b64_stream(stdin, stdout);
	if (status != HEX2B64_OK) {
		fprintf(stderr, "hex2b64: %s\n",
		    hex2b64_status_string(status));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
