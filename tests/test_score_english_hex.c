/**
 * @file test_score_english_hex.c
 * @brief Unit tests for the English scoring heuristic.
 */

#include "score_english_hex.h"
#include "utest.h"

UTEST(score_english_hex, english_phrase_beats_random) {
    const char *english_hex = "54686520717569636b2062726f776e20666f7820";
    const char *random_hex = "9f4c3ad2b7e18c44ff00aa11cc33";
    double english_score = score_english_hex(english_hex);
    double random_score = score_english_hex(random_hex);
    ASSERT_GT(english_score, random_score);
}

UTEST(score_english_hex, rejects_invalid_hex_character) {
    const char *hex = "zz";
    double score = score_english_hex(hex);
    ASSERT_LT(score, -1e8);
}

UTEST(score_english_hex, rejects_odd_length) {
    const char *hex = "414";
    double score = score_english_hex(hex);
    ASSERT_LT(score, -1e8);
}

UTEST(score_english_hex, rejects_non_printable_bytes) {
    const char *hex = "0001";
    double score = score_english_hex(hex);
    ASSERT_LT(score, 0.0);
    ASSERT_GT(score, -1e8);
}

UTEST_MAIN();
