/*
 * Unit tests for the hex2b64 conversion helpers.
 */

#include <stdio.h>
#include <string.h>

#include "hex2b64.h"
#include "utest.h"

/*
 * Run hex2b64_stream() over an in-memory string and capture its output in out.
 */
static int convert_hex_string(const char *hex, char *out, size_t out_cap) {
    FILE *in = tmpfile();
    FILE *outf = tmpfile();
    if (!in || !outf) {
        if (in) fclose(in);
        if (outf) fclose(outf);
        return -1;
    }

    fputs(hex, in);
    rewind(in);

    int rc = hex2b64_stream(in, outf);

    fflush(outf);
    rewind(outf);

    size_t n = fread(out, 1, out_cap - 1, outf);
    out[n] = '\0';

    fclose(in);
    fclose(outf);

    return rc;
}

/*
 * Convenience wrapper around hex2b64_buffer() for string literals.
 */
static int convert_hex_buffer(const char *hex,
                              unsigned char *out,
                              size_t out_cap,
                              size_t *out_len) {
    return hex2b64_buffer((const unsigned char *)hex,
                          strlen(hex),
                          out,
                          out_cap,
                          out_len);
}

/* === Integration tests for hex2b64_stream (normal cases) === */

UTEST(stream, hello_world_plain) {
    char out[128];
    int rc = convert_hex_string("48656c6c6f20776f726c64", out, sizeof(out));
    ASSERT_EQ(0, rc);
    /* "Hello world" in Base64 plus newline */
    ASSERT_STREQ("SGVsbG8gd29ybGQ=\n", out);
}

UTEST(stream, ignores_whitespace) {
    char out[128];
    int rc = convert_hex_string("48 65 6c 6c 6f 20 77 6f 72 6c 64\n",
                                out, sizeof(out));
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("SGVsbG8gd29ybGQ=\n", out);
}

/* Mixed upper/lowercase hex for same "Hello world". */
UTEST(stream, mixed_case_hex) {
    char out[128];
    int rc = convert_hex_string("48656C6c6F20776F726C64", out, sizeof(out));
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("SGVsbG8gd29ybGQ=\n", out);
}

/* === Integration tests: edge cases === */

/* Empty input: should just produce a newline. */
UTEST(stream_edge, empty_input) {
    char out[128];
    int rc = convert_hex_string("", out, sizeof(out));
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("\n", out);
}

/* Only whitespace: also just newline. */
UTEST(stream_edge, whitespace_only) {
    char out[128];
    int rc = convert_hex_string("   \n\t  ", out, sizeof(out));
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("\n", out);
}

/* Single byte (2 hex digits) -> 1-byte Base64 with padding. "M" -> "TQ==" */
UTEST(stream_edge, single_byte) {
    char out[128];
    int rc = convert_hex_string("4d", out, sizeof(out)); /* 'M' */
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("TQ==\n", out);
}

/* Two bytes (4 hex digits) -> 2-byte Base64 + '=' padding. "Ma" -> "TWE=" */
UTEST(stream_edge, two_bytes) {
    char out[128];
    int rc = convert_hex_string("4d61", out, sizeof(out)); /* "Ma" */
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("TWE=\n", out);
}

/* Three bytes (6 hex digits) -> 3-byte Base64, no padding. "Man" -> "TWFu" */
UTEST(stream_edge, three_bytes) {
    char out[128];
    int rc = convert_hex_string("4d616e", out, sizeof(out)); /* "Man" */
    ASSERT_EQ(0, rc);
    ASSERT_STREQ("TWFu\n", out);
}

/* Odd number of hex digits: error. */
UTEST(stream_edge, odd_number_of_digits_is_error) {
    char out[128];
    int rc = convert_hex_string("48656c6c6f7", out, sizeof(out));
    ASSERT_NE(0, rc);   /* Should fail */
}

/* Invalid hex character in the middle: error. */
UTEST(stream_edge, invalid_char_in_middle) {
    char out[128];
    int rc = convert_hex_string("48656x6c6f", out, sizeof(out));
    ASSERT_NE(0, rc);   /* Should fail */
}

/* Invalid hex character as first non-whitespace: error. */
UTEST(stream_edge, invalid_first_char) {
    char out[128];
    int rc = convert_hex_string("z8656c6c6f", out, sizeof(out));
    ASSERT_NE(0, rc);   /* Should fail */
}

/* Large-ish input: many 'A' bytes (0x41) – just sanity check result shape. */
UTEST(stream_edge, many_bytes_sanity) {
    char hex[1024];
    char out[2048];

    /* 100 bytes of 'A' (0x41 => "41" in hex). */
    size_t i;
    for (i = 0; i < 100; ++i) {
        hex[2 * i]     = '4';
        hex[2 * i + 1] = '1';
    }
    hex[2 * 100] = '\0';

    int rc = convert_hex_string(hex, out, sizeof(out));
    ASSERT_EQ(0, rc);

    /* Just basic sanity: output must end with '\n' and be non-empty. */
    size_t len = strlen(out);
    ASSERT_GT(len, 1u);
    ASSERT_EQ('\n', out[len - 1]);
}

/* === Tests for hex2b64_buffer === */

UTEST(buffer, hello_world_plain) {
    unsigned char out[128];
    size_t out_len = 0;
    int rc = convert_hex_buffer("48656c6c6f20776f726c64",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(strlen("SGVsbG8gd29ybGQ=\n"), out_len);
    out[out_len] = '\0';
    ASSERT_STREQ("SGVsbG8gd29ybGQ=\n", (const char *)out);
}

UTEST(buffer, ignores_whitespace) {
    unsigned char out[128];
    size_t out_len = 0;
    int rc = convert_hex_buffer("48 65 6c 6c 6f",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_EQ(0, rc);
    out[out_len] = '\0';
    ASSERT_STREQ("SGVsbG8=\n", (const char *)out);
}

UTEST(buffer, empty_input) {
    unsigned char out[8];
    size_t out_len = 0;
    int rc = convert_hex_buffer("",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(1u, out_len);
    out[out_len] = '\0';
    ASSERT_STREQ("\n", (const char *)out);
}

UTEST(buffer_edge, odd_digits_error) {
    unsigned char out[16];
    size_t out_len = 0;
    int rc = convert_hex_buffer("123",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_NE(0, rc);
}

UTEST(buffer_edge, invalid_character) {
    unsigned char out[16];
    size_t out_len = 0;
    int rc = convert_hex_buffer("48xx",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_NE(0, rc);
}

UTEST(buffer_edge, insufficient_output_capacity) {
    unsigned char out[4];
    size_t out_len = 0;
    int rc = convert_hex_buffer("4d",
                                out,
                                sizeof(out),
                                &out_len);
    ASSERT_NE(0, rc);
}

/* Let utest.h provide main(). */
UTEST_MAIN();
