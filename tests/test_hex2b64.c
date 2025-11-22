#include <stdio.h>
#include <string.h>

#include "hex2b64.h"
#include "utest.h"

/* Helper: encode one block into an in-memory string. */
static void encode_block_to_string(const unsigned char *in,
                                   size_t len,
                                   char out[5],
                                   int *utest_result) {
    FILE *f = tmpfile();
    ASSERT_TRUE(f != NULL);

    encode_base64_block(in, len, f);

    fflush(f);
    rewind(f);

    size_t n = fread(out, 1, 4, f);
    ASSERT_EQ(4u, n);

    out[4] = '\0';
    fclose(f);
}

/* Helper: run hex2b64_stream on a string, capture result into out. */
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

/* === Tests for hex_value === */

UTEST(hex_value, decimal_digits) {
    EXPECT_EQ(0, hex_value('0'));
    EXPECT_EQ(1, hex_value('1'));
    EXPECT_EQ(9, hex_value('9'));
}

UTEST(hex_value, lowercase_letters) {
    EXPECT_EQ(10, hex_value('a'));
    EXPECT_EQ(11, hex_value('b'));
    EXPECT_EQ(15, hex_value('f'));
}

UTEST(hex_value, uppercase_letters) {
    EXPECT_EQ(10, hex_value('A'));
    EXPECT_EQ(11, hex_value('B'));
    EXPECT_EQ(15, hex_value('F'));
}

UTEST(hex_value, invalid_chars) {
    EXPECT_EQ(-1, hex_value('g'));
    EXPECT_EQ(-1, hex_value('G'));
    EXPECT_EQ(-1, hex_value('x'));
    EXPECT_EQ(-1, hex_value(' '));
    EXPECT_EQ(-1, hex_value('\n'));
}

/* === Tests for encode_base64_block === */

UTEST(base64_block, three_bytes) {
    const unsigned char in[3] = { 'M', 'a', 'n' }; /* "Man" -> "TWFu" */
    char out[5];
    encode_block_to_string(in, 3, out, utest_result);
    ASSERT_STREQ("TWFu", out);
}

UTEST(base64_block, two_bytes) {
    const unsigned char in[2] = { 'M', 'a' }; /* "Ma" -> "TWE=" */
    char out[5];
    encode_block_to_string(in, 2, out, utest_result);
    ASSERT_STREQ("TWE=", out);
}

UTEST(base64_block, one_byte) {
    const unsigned char in[1] = { 'M' }; /* "M" -> "TQ==" */
    char out[5];
    encode_block_to_string(in, 1, out, utest_result);
    ASSERT_STREQ("TQ==", out);
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

/* Let utest.h provide main(). */
UTEST_MAIN();
