#include "solution.hpp"
#include <cstdint>
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>

// Find the longest line in a file.
// Implementation uses ternary operator with a hope that compiler will
// turn it into a CMOV instruction.
// The code inside the inner loop is equivalent to:
/*
if (s == '\n') {
  longestLine = std::max(curLineLength, longestLine);
  curLineLength = 0;
} else {
  curLineLength++;
}*/
unsigned solution(const std::string &inputContents) {
  unsigned longestLine = 0;
  unsigned curLineLength = 0;

  /// Origin version:
  // for (auto s : inputContents) {
  //   curLineLength = (s == '\n') ? 0 : curLineLength + 1;
  //   longestLine = std::max(curLineLength, longestLine);
  // }

  /// SIMD version:
  const char *inputAddr = inputContents.data();
  int i = 0;
  // Stores the length of a line segment that carries over from the preLineLength chunk.
  int preLineLength = 0;
  for (; i + 15 < inputContents.size(); i += 16) {
    // Load 16 characters (unaligned) from the input string into a 128-bit SIMD register.
    __m128i charV = _mm_loadu_si128((const __m128i *)(inputAddr + i));
    // Create a 128-bit vector where all 16 bytes are the newline character ('\n').
    __m128i LFMask = _mm_set1_epi8('\n');

    // Compare each character in our input vector with the newline character.
    // The resulting vector's bytes will be 0xFF if the characters match, and 0x00 otherwise.
    __m128i result = _mm_cmpeq_epi8(charV, LFMask);
    // Create a 16-bit integer mask from the result. Each '1' in this mask
    // represents the position of a '\n' in the 16-byte chunk.
    uint32_t uint_reuslt = _mm_movemask_epi8(result);

    int length = 0;
    // Loop as long as there are newlines (set bits) in the current chunk's mask.
    while (uint_reuslt) {
      int maskIdx = _tzcnt_u32(uint_reuslt);
      // Calculate the full line length by adding the carry-over from the preLineLength chunk (`preLineLength`)
      // and the index of the current newline (`_tzcnt_u32`).
      curLineLength += (maskIdx + preLineLength);

      // Update 'length' by the number of characters in the line segment just processed.
      length += maskIdx + 1;

      // Update the overall maximum line length found so far.
      longestLine = std::max(curLineLength, longestLine);

      // Set variables for the next potential line found within this same chunk.
      uint_reuslt >>= (maskIdx + 1);
      curLineLength = 0;
      preLineLength = 0;
    }
    // After the loop, calculate the length of the line segment remaining at the end of the chunk.
    // This value is carried over to the next chunk via `preLineLength`.
    preLineLength += 16 - length;
  }

  // This final scalar loop handles any remaining characters that didn't form a full 16-byte chunk.
  // Initialize the current line length with the carry-over from the last SIMD chunk.
  curLineLength = preLineLength;
  for (; i < inputContents.size(); i++) {
    curLineLength = (inputContents[i] == '\n') ? 0 : curLineLength + 1;
    longestLine = std::max(curLineLength, longestLine);
  }

  return longestLine;
}
