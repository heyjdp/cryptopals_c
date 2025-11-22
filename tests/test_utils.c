/**
 * @file test_utils.c
 * @brief Unit tests for shared utility helpers.
 */

#include <stdint.h>
#include <string.h>

#include "fixed_xor.h"
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
	size_t len = 0;
	utils_status status = hex_to_bytes("deadbeef", out, sizeof(out), &len);
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_EQ(4u, len);
	EXPECT_EQ(0xDE, out[0]);
	EXPECT_EQ(0xAD, out[1]);
	EXPECT_EQ(0xBE, out[2]);
	EXPECT_EQ(0xEF, out[3]);
}

UTEST(hex_to_bytes, rejects_invalid_hex)
{
	uint8_t out[2];
	size_t len = 0;
	utils_status status = hex_to_bytes("zz", out, sizeof(out), &len);
	ASSERT_EQ(UTILS_ERR_INVALID_HEX, status);
}

UTEST(hex_to_bytes, rejects_odd_length)
{
	uint8_t out[2];
	utils_status status = hex_to_bytes("abc", out, sizeof(out), NULL);
	ASSERT_EQ(UTILS_ERR_ODD_LENGTH, status);
}

UTEST(hex_to_bytes, detects_small_buffer)
{
	uint8_t out[1];
	utils_status status = hex_to_bytes("abcd", out, sizeof(out), NULL);
	ASSERT_EQ(UTILS_ERR_BUFFER_TOO_SMALL, status);
}

UTEST(hex_to_bytes, rejects_null_buffer)
{
	utils_status status = hex_to_bytes("aa", NULL, 1, NULL);
	ASSERT_EQ(UTILS_ERR_ARGS, status);
}

UTEST(bytes_to_hex, round_trip)
{
	const uint8_t bytes[] = { 0x41, 0x42, 0x43 };
	char hex[7];
	utils_status status =
	    bytes_to_hex(bytes, sizeof(bytes), hex, sizeof(hex));
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_STREQ("414243", hex);
}

UTEST(bytes_to_hex, buffer_too_small)
{
	const uint8_t bytes[] = { 0x41 };
	char hex[2];
	utils_status status =
	    bytes_to_hex(bytes, sizeof(bytes), hex, sizeof(hex));
	ASSERT_EQ(UTILS_ERR_BUFFER_TOO_SMALL, status);
}

UTEST(hex_to_ascii, converts_string)
{
	char out[4];
	utils_status status = hex_to_ascii("414243", out, sizeof(out));
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_STREQ("ABC", out);
}

UTEST(hex_to_ascii, truncates_when_short)
{
	char out[3];
	utils_status status = hex_to_ascii("414243", out, sizeof(out));
	ASSERT_EQ(UTILS_ERR_BUFFER_TOO_SMALL, status);
}

UTEST(brute_force_single_byte_xor, decodes_known_cipher)
{
	const char hex[] =
	    "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";
	uint8_t plain[128];
	size_t len = 0;
	uint8_t key = 0;
	double score = 0.0;

	utils_status status = brute_force_single_byte_xor(hex,
	    plain, sizeof(plain), &len, &key, &score);
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_GT(score, -100.0);
	ASSERT_NE(0u, len);

	char ascii[128];
	char encoded[256];
	ASSERT_EQ(UTILS_OK,
	    bytes_to_hex(plain, len, encoded, sizeof(encoded)));
	ASSERT_EQ(UTILS_OK, hex_to_ascii(encoded, ascii, sizeof(ascii)));
	ASSERT_STREQ("Cooking MC's like a pound of bacon", ascii);
}

UTEST(brute_force_single_byte_xor, invalid_hex_input)
{
	uint8_t plain[8];
	size_t len = 0;
	uint8_t key = 0;
	const char bad_hex[] = "zz";
	utils_status status = brute_force_single_byte_xor(bad_hex,
	    plain, sizeof(plain), &len, &key, NULL);
	ASSERT_EQ(UTILS_ERR_INVALID_HEX, status);
}

UTEST(brute_force_single_byte_xor, odd_length_input)
{
	uint8_t plain[8];
	size_t len = 0;
	uint8_t key = 0;
	const char odd_hex[] = "123";
	utils_status status = brute_force_single_byte_xor(odd_hex,
	    plain, sizeof(plain), &len, &key, NULL);
	ASSERT_EQ(UTILS_ERR_ODD_LENGTH, status);
}

UTEST(brute_force_single_byte_xor, insufficient_output_buffer)
{
	const char hex[] = "abcd";
	uint8_t plain[1];
	size_t len = 0;
	uint8_t key = 0;
	utils_status status = brute_force_single_byte_xor(hex,
	    plain, sizeof(plain), &len, &key, NULL);
	ASSERT_EQ(UTILS_ERR_BUFFER_TOO_SMALL, status);
}

UTEST(utils_repeat_key, fills_buffer)
{
	const char key[] = "ICE";
	uint8_t out[8];
	utils_status status = utils_repeat_key(key, out, sizeof(out));
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_EQ('I', out[0]);
	ASSERT_EQ('C', out[1]);
	ASSERT_EQ('E', out[2]);
	ASSERT_EQ('I', out[3]);
}

UTEST(utils_repeat_key, zero_length_output_ok)
{
	const char key[] = "ICE";
	utils_status status = utils_repeat_key(key, NULL, 0);
	ASSERT_EQ(UTILS_OK, status);
}

UTEST(utils_repeat_key, rejects_empty_key)
{
	uint8_t out[4];
	utils_status status = utils_repeat_key("", out, sizeof(out));
	ASSERT_EQ(UTILS_ERR_ARGS, status);
}

UTEST(utils_repeat_key, encrypts_expected_cipher)
{
	const char plaintext[] =
	    "Burning 'em, if you ain't quick and nimble\n"
	    "I go crazy when I hear a cymbal";
	const char expected_hex[] =
	    "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324"
	    "272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b2028316528"
	    "6326302e27282f";

	size_t plaintext_len = strlen(plaintext);
	uint8_t *key_stream = malloc(plaintext_len);
	uint8_t *cipher = malloc(plaintext_len);
	char *cipher_hex = malloc(plaintext_len * 2 + 1);
	ASSERT_TRUE(key_stream != NULL);
	ASSERT_TRUE(cipher != NULL);
	ASSERT_TRUE(cipher_hex != NULL);

	utils_status status =
	    utils_repeat_key("ICE", key_stream, plaintext_len);
	ASSERT_EQ(UTILS_OK, status);

	fixed_xor_status xor_status =
	    fixed_xor_buffers((const uint8_t *) plaintext,
	    key_stream, cipher, plaintext_len);
	ASSERT_EQ(FIXED_XOR_OK, xor_status);

	status =
	    bytes_to_hex(cipher,
	    plaintext_len, cipher_hex, plaintext_len * 2 + 1);
	ASSERT_EQ(UTILS_OK, status);
	ASSERT_STREQ(expected_hex, cipher_hex);

	free(key_stream);
	free(cipher);
	free(cipher_hex);
}

UTEST(utils_status_string, returns_messages)
{
	ASSERT_STREQ("success", utils_status_string(UTILS_OK));
	ASSERT_STREQ("invalid arguments", utils_status_string(UTILS_ERR_ARGS));
	ASSERT_STREQ("invalid hex digit",
	    utils_status_string(UTILS_ERR_INVALID_HEX));
	ASSERT_STREQ("odd number of hex digits",
	    utils_status_string(UTILS_ERR_ODD_LENGTH));
	ASSERT_STREQ("output buffer too small",
	    utils_status_string(UTILS_ERR_BUFFER_TOO_SMALL));
	ASSERT_STREQ("out of memory", utils_status_string(UTILS_ERR_OOM));
	ASSERT_STREQ("failed to score english text",
	    utils_status_string(UTILS_ERR_SCORE_FAIL));
}

UTEST_MAIN();
