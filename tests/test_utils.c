/**
 * @file test_utils.c
 * @brief Unit tests for shared utility helpers.
 */

#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "utest.h"

UTEST(hex_digit_value, valid_digits)
{
	EXPECT_EQ(0, hex_digit_value('0'));
	EXPECT_EQ(9, hex_digit_value('9'));
	EXPECT_EQ(10, hex_digit_value('a'));
	EXPECT_EQ(15, hex_digit_value('f'));
	EXPECT_EQ(10, hex_digit_value('A'));
	EXPECT_EQ(15, hex_digit_value('F'));
}

UTEST(hex_digit_value, invalid_digits)
{
	EXPECT_EQ(-1, hex_digit_value('g'));
	EXPECT_EQ(-1, hex_digit_value('Z'));
	EXPECT_EQ(-1, hex_digit_value(' '));
}

UTEST(hex_to_bytes, converts_string)
{
	uint8_t out[4];
	int len = hex_to_bytes("deadbeef", out, sizeof(out));
	ASSERT_EQ(4, len);
	EXPECT_EQ(0xDE, out[0]);
	EXPECT_EQ(0xAD, out[1]);
	EXPECT_EQ(0xBE, out[2]);
	EXPECT_EQ(0xEF, out[3]);
}

UTEST(hex_to_bytes, rejects_invalid_hex)
{
	uint8_t out[2];
	EXPECT_EQ(-1, hex_to_bytes("zz", out, sizeof(out)));
}

UTEST(hex_to_bytes, rejects_odd_length)
{
	uint8_t out[2];
	EXPECT_EQ(-1, hex_to_bytes("abc", out, sizeof(out)));
}

UTEST(hex_to_bytes, detects_small_buffer)
{
	uint8_t out[1];
	EXPECT_EQ(-1, hex_to_bytes("abcd", out, sizeof(out)));
}

UTEST(bytes_to_hex, round_trip)
{
	const uint8_t bytes[] = { 0x41, 0x42, 0x43 };
	char hex[7];
	bytes_to_hex(bytes, sizeof(bytes), hex);
	ASSERT_STREQ("414243", hex);
}

UTEST(hex_to_ascii, converts_string)
{
	char out[4];
	hex_to_ascii("414243", out, sizeof(out));
	ASSERT_STREQ("ABC", out);
}

UTEST(hex_to_ascii, truncates_when_short)
{
	char out[3];
	hex_to_ascii("414243", out, sizeof(out));
	ASSERT_STREQ("AB", out);
}

UTEST(brute_force_single_byte_xor, decodes_known_cipher)
{
	const char hex[] =
	    "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";
	uint8_t plain[128];
	size_t len = 0;
	uint8_t key = 0;
	double score = 0.0;

	int rc = brute_force_single_byte_xor(hex, plain, &len, &key, &score);
	ASSERT_EQ(0, rc);
	ASSERT_GT(score, -100.0);
	ASSERT_NE(0u, len);

	char ascii[128];
	char encoded[256];
	bytes_to_hex(plain, len, encoded);
	hex_to_ascii(encoded, ascii, sizeof(ascii));
	ASSERT_STREQ("Cooking MC's like a pound of bacon", ascii);
}

UTEST(brute_force_single_byte_xor, invalid_input)
{
	uint8_t plain[8];
	size_t len = 0;
	uint8_t key = 0;
	const char bad_hex[] = "zzz";
	EXPECT_NE(0, brute_force_single_byte_xor(bad_hex, plain, &len, &key,
		NULL));
}

UTEST_MAIN();
