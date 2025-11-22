/**
 * @file test_score_english_hex.c
 * @brief Unit tests for score_english_hex().
 */

#include "score_english_hex.h"
#include "utest.h"

UTEST(score_english_hex, english_phrase_beats_random)
{
	const char english_hex[] = "54686520717569636b2062726f776e20666f7820";
	const char random_hex[] = "9f4c3ad2b7e18c44ff00aa11cc33";

	double english_score = 0.0;
	double random_score = 0.0;
	ASSERT_EQ(SCORE_ENGLISH_HEX_OK,
	    score_english_hex(english_hex, &english_score));
	ASSERT_EQ(SCORE_ENGLISH_HEX_OK,
	    score_english_hex(random_hex, &random_score));
	ASSERT_GT(english_score, random_score);
}

UTEST(score_english_hex, rejects_invalid_hex_character)
{
	double score = 0.0;
	score_english_hex_status status = score_english_hex("zz", &score);
	ASSERT_EQ(SCORE_ENGLISH_HEX_ERR_INVALID_HEX, status);
}

UTEST(score_english_hex, rejects_odd_length)
{
	double score = 0.0;
	score_english_hex_status status = score_english_hex("414", &score);
	ASSERT_EQ(SCORE_ENGLISH_HEX_ERR_ODD_LENGTH, status);
}

UTEST(score_english_hex, rejects_empty_string)
{
	double score = 0.0;
	score_english_hex_status status = score_english_hex("", &score);
	ASSERT_EQ(SCORE_ENGLISH_HEX_ERR_EMPTY, status);
}

UTEST(score_english_hex, rejects_null_arguments)
{
	score_english_hex_status status = score_english_hex(NULL, NULL);
	ASSERT_EQ(SCORE_ENGLISH_HEX_ERR_ARGS, status);
}

UTEST_MAIN();
