#ifndef SCORE_ENGLISH_HEX_H
#define SCORE_ENGLISH_HEX_H

/**
 * @file score_english_hex.h
 * @brief Estimate how closely hex-encoded data resembles English text.
 */

/**
 * @brief Score a hex string based on English letter frequency heuristics.
 *
 * @param hex Null-terminated ASCII hex string.
 * @return Higher values indicate closer resemblance to English text.
 */
double score_english_hex(const char *hex);

#endif /* SCORE_ENGLISH_HEX_H */
