#include <stdio.h>
#include <string.h>

#include "fixed_xor.h"
#include "utest.h"

UTEST(fixed_xor_buffers, simple_values) {
    const unsigned char lhs[] = {0x00, 0xFF, 0x10, 0x55};
    const unsigned char rhs[] = {0x00, 0x0F, 0x10, 0xAA};
    unsigned char out[sizeof(lhs)];

    fixed_xor_status status =
        fixed_xor_buffers(lhs, rhs, out, sizeof(lhs));
    ASSERT_EQ(FIXED_XOR_OK, status);

    const unsigned char expected[] = {0x00, 0xF0, 0x00, 0xFF};
    ASSERT_EQ(0, memcmp(out, expected, sizeof(expected)));
}

UTEST(fixed_xor_buffers, zero_length_is_ok) {
    const unsigned char lhs = 0xAB;
    const unsigned char rhs = 0xCD;
    unsigned char out = 0;

    fixed_xor_status status = fixed_xor_buffers(&lhs, &rhs, &out, 0);
    ASSERT_EQ(FIXED_XOR_OK, status);
}

UTEST(fixed_xor_buffers, allows_output_alias) {
    unsigned char data[] = {0x0F, 0xF0, 0xAA, 0x55};
    const unsigned char rhs[] = {0xFF, 0x00, 0xAA, 0x55};

    fixed_xor_status status =
        fixed_xor_buffers(data, rhs, data, sizeof(data));
    ASSERT_EQ(FIXED_XOR_OK, status);

    const unsigned char expected[] = {0xF0, 0xF0, 0x00, 0x00};
    ASSERT_EQ(0, memcmp(data, expected, sizeof(expected)));
}

UTEST(fixed_xor_buffers, null_pointer_rejected) {
    unsigned char lhs = 0x01;
    unsigned char rhs = 0x02;
    unsigned char out = 0x00;

    fixed_xor_status status = fixed_xor_buffers(&lhs, NULL, &out, 1);
    ASSERT_EQ(FIXED_XOR_ERR_ARGS, status);

    status = fixed_xor_buffers(NULL, &rhs, &out, 1);
    ASSERT_EQ(FIXED_XOR_ERR_ARGS, status);

    status = fixed_xor_buffers(&lhs, &rhs, NULL, 1);
    ASSERT_EQ(FIXED_XOR_ERR_ARGS, status);
}

UTEST(fixed_xor_stream, processes_two_buffers) {
    FILE *in = tmpfile();
    FILE *out = tmpfile();
    ASSERT_TRUE(in != NULL);
    ASSERT_TRUE(out != NULL);

    const unsigned char lhs[] = {0x01, 0x23, 0x45, 0x67};
    const unsigned char rhs[] = {0x89, 0xAB, 0xCD, 0xEF};
    ASSERT_EQ(sizeof(lhs), fwrite(lhs, 1, sizeof(lhs), in));
    ASSERT_EQ(sizeof(rhs), fwrite(rhs, 1, sizeof(rhs), in));
    rewind(in);

    fixed_xor_status status = fixed_xor_stream(in, out);
    ASSERT_EQ(FIXED_XOR_OK, status);

    rewind(out);
    unsigned char buf[sizeof(lhs)];
    ASSERT_EQ(sizeof(buf), fread(buf, 1, sizeof(buf), out));

    const unsigned char expected[] = {0x88, 0x88, 0x88, 0x88};
    ASSERT_EQ(0, memcmp(buf, expected, sizeof(expected)));

    fclose(in);
    fclose(out);
}

UTEST(fixed_xor_stream, empty_input_ok) {
    FILE *in = tmpfile();
    FILE *out = tmpfile();
    ASSERT_TRUE(in != NULL);
    ASSERT_TRUE(out != NULL);

    fixed_xor_status status = fixed_xor_stream(in, out);
    ASSERT_EQ(FIXED_XOR_OK, status);

    rewind(out);
    unsigned char buf[1];
    ASSERT_EQ(0u, fread(buf, 1, sizeof(buf), out));

    fclose(in);
    fclose(out);
}

UTEST(fixed_xor_stream, odd_length_input_fails) {
    FILE *in = tmpfile();
    FILE *out = tmpfile();
    ASSERT_TRUE(in != NULL);
    ASSERT_TRUE(out != NULL);

    const unsigned char data[] = {0xAA, 0xBB, 0xCC};
    ASSERT_EQ(sizeof(data), fwrite(data, 1, sizeof(data), in));
    rewind(in);

    fixed_xor_status status = fixed_xor_stream(in, out);
    ASSERT_EQ(FIXED_XOR_ERR_ODD_INPUT, status);

    fclose(in);
    fclose(out);
}

UTEST(fixed_xor_stream, large_input_triggers_growth) {
    enum { SAMPLE = 6000 };
    unsigned char lhs[SAMPLE];
    unsigned char rhs[SAMPLE];
    unsigned char expected[SAMPLE];
    for (int i = 0; i < SAMPLE; ++i) {
        lhs[i] = (unsigned char)i;
        rhs[i] = (unsigned char)(SAMPLE - i);
        expected[i] = lhs[i] ^ rhs[i];
    }

    FILE *in = tmpfile();
    FILE *out = tmpfile();
    ASSERT_TRUE(in != NULL);
    ASSERT_TRUE(out != NULL);

    ASSERT_EQ((size_t)SAMPLE, fwrite(lhs, 1, SAMPLE, in));
    ASSERT_EQ((size_t)SAMPLE, fwrite(rhs, 1, SAMPLE, in));
    rewind(in);

    fixed_xor_status status = fixed_xor_stream(in, out);
    ASSERT_EQ(FIXED_XOR_OK, status);

    rewind(out);
    unsigned char buf[SAMPLE];
    ASSERT_EQ((size_t)SAMPLE, fread(buf, 1, SAMPLE, out));
    ASSERT_EQ(0, memcmp(buf, expected, SAMPLE));

    fclose(in);
    fclose(out);
}

UTEST(fixed_xor_status_string, returns_text) {
    ASSERT_STREQ("success", fixed_xor_status_string(FIXED_XOR_OK));
    ASSERT_STREQ("invalid arguments", fixed_xor_status_string(FIXED_XOR_ERR_ARGS));
    ASSERT_STREQ("I/O failure", fixed_xor_status_string(FIXED_XOR_ERR_IO));
    ASSERT_STREQ("input length must be even (two equal buffers)",
                 fixed_xor_status_string(FIXED_XOR_ERR_ODD_INPUT));
    ASSERT_STREQ("out of memory",
                 fixed_xor_status_string(FIXED_XOR_ERR_OOM));
}

UTEST_MAIN();
