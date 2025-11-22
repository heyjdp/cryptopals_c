/**
 * @file hex2b64_main.c
 * @brief Command-line tool that converts hex input to Base64.
 */

#include <stdio.h>
#include "hex2b64.h"

/**
 * @brief Entry point that wires stdin/stdout through hex2b64_stream().
 *
 * @return 0 on success, non-zero if conversion fails.
 */
int
main(void)
{
	return hex2b64_stream(stdin, stdout);
}
