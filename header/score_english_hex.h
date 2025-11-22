#ifndef SCORE_ENGLISH_HEX_H
#define SCORE_ENGLISH_HEX_H

/**
 * @file score_english_hex.h
 * @brief Estimate how closely hex-encoded data resembles English text.
 */

typedef enum
{
	SCORE_ENGLISH_HEX_OK = 0,
	SCORE_ENGLISH_HEX_ERR_ARGS = -1,
	SCORE_ENGLISH_HEX_ERR_EMPTY = -2,
	SCORE_ENGLISH_HEX_ERR_ODD_LENGTH = -3,
	SCORE_ENGLISH_HEX_ERR_INVALID_HEX = -4
} score_english_hex_status;

score_english_hex_status score_english_hex(const char *hex, double *score_out);

const char *score_english_hex_status_string(score_english_hex_status status);

#endif /* SCORE_ENGLISH_HEX_H */
