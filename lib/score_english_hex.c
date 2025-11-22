#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * English letter frequencies (a-z + space) as proportions.
 * Roughly based on typical English text frequency analyses.
 * They should sum to about 1.0.
 *
 * Index 0-25: 'a'-'z'
 * Index 26:   ' '
 */
static const double english_freq[27] = {
    0.0817, // a
    0.0150, // b
    0.0278, // c
    0.0425, // d
    0.1270, // e
    0.0223, // f
    0.0202, // g
    0.0609, // h
    0.0697, // i
    0.0015, // j
    0.0077, // k
    0.0403, // l
    0.0241, // m
    0.0675, // n
    0.0751, // o
    0.0193, // p
    0.0010, // q
    0.0599, // r
    0.0633, // s
    0.0906, // t
    0.0276, // u
    0.0098, // v
    0.0236, // w
    0.0015, // x
    0.0197, // y
    0.0007, // z
    0.1300  // space (very common in normal English text)
};

/* Convert a single hex character to its integer value, or -1 on error. */
static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

/*
 * Score how "English-like" a hex-encoded ASCII string is.
 * Higher scores are better.
 *
 * Returns a large negative number on invalid hex or clearly non-English bytes.
 */
double score_english_hex(const char *hex) {
    size_t hex_len = strlen(hex);
    if (hex_len == 0 || (hex_len & 1)) {
        // empty or odd-length hex is invalid
        return -1e9;
    }

    int counts[27] = {0};  // letter+space counts
    size_t total_letters = 0;
    size_t total_bytes = 0;
    double penalty = 0.0;

    // Decode hex on the fly; no need to allocate a separate buffer.
    for (size_t i = 0; i < hex_len; i += 2) {
        int hi = hex_value(hex[i]);
        int lo = hex_value(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            // invalid hex character
            return -1e9;
        }

        unsigned char c = (unsigned char)((hi << 4) | lo);
        total_bytes++;

        if (isalpha(c)) {
            c = (unsigned char)tolower(c);
            counts[c - 'a']++;
            total_letters++;
        } else if (c == ' ') {
            counts[26]++;
            total_letters++;
        } else if (c == '\n' || c == '\r' || c == '\t' ||
                   c == ','  || c == '.'  || c == '\'' || c == '"') {
            // Neutral punctuation/whitespace: allowed but not counted as letters.
        } else if (c < 32 || c > 126) {
            // Non-printable or non-ASCII characters: penalize heavily but keep scoring.
            penalty += 50.0;
            continue;
        } else {
            // Other printable symbols like ! ? ; : etc. are allowed but not counted as letters.
        }
    }

    if (total_bytes == 0) {
        return -1e9;
    }
    if (total_letters == 0) {
        return -1000.0 - penalty;
    }

    // Compute a chi-squared style statistic over letters+space.
    // Lower chi2 means closer to English; we will invert it into a score.
    double chi2 = 0.0;
    for (int i = 0; i < 27; i++) {
        double expected = english_freq[i] * (double)total_letters;
        double observed = (double)counts[i];
        double diff = observed - expected;
        // Add a tiny constant to avoid division by zero.
        chi2 += (diff * diff) / (expected + 1e-9);
    }

    // Convert chi-squared to a score.
    // Smaller chi2 -> higher score. Add bonus for high proportion of letters/spaces.
    double letter_ratio = (double)total_letters / (double)total_bytes;
    double score = -chi2 + letter_ratio * 50.0 - penalty;

    return score;
}
