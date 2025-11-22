#include <stdio.h>
#include "hex2b64.h"

int main(void) {
    return hex2b64_stream(stdin, stdout);
}
