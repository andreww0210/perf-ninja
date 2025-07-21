#include "solution.h"
#include <cstdint>
#include <emmintrin.h>
#include <immintrin.h>
#include <memory>

// void imageSmoothing(const InputVector &input, uint8_t radius,
//                     OutputVector &output) {
//   int pos = 0;
//   int currentSum = 0;
//   int size = static_cast<int>(input.size());

//   // 1. left border - time spend in this loop can be ignored, no need to
//   // optimize it
//   for (int i = 0; i < std::min<int>(size, radius); ++i) {
//     currentSum += input[i];
//   }

//   int limit = std::min(radius + 1, size - radius);
//   for (pos = 0; pos < limit; ++pos) {
//     currentSum += input[pos + radius];
//     output[pos] = currentSum;
//   }

//   // 2. main loop.
//   limit = size - radius;
//   for (; pos < limit; ++pos) {
//     currentSum -= input[pos - radius - 1];
//     currentSum += input[pos + radius];
//     output[pos] = currentSum;
//   }

//   // 3. special case, executed only if size <= 2*radius + 1
//   limit = std::min(radius + 1, size);
//   for (; pos < limit; pos++) {
//     output[pos] = currentSum;
//   }

//   // 4. right border - time spend in this loop can be ignored, no need to
//   // optimize it
//   for (; pos < size; ++pos) {
//     currentSum -= input[pos - radius - 1];
//     output[pos] = currentSum;
//   }
// }

void imageSmoothing(const InputVector &input, uint8_t radius,
                    OutputVector &output) {
  int pos = 0;
  int currentSum = 0;
  int size = static_cast<int>(input.size());

  // 1. left border - time spend in this loop can be ignored, no need to
  // optimize it
  for (int i = 0; i < std::min<int>(size, radius); ++i) {
    currentSum += input[i];
  }

  int limit = std::min(radius + 1, size - radius);
  for (pos = 0; pos < limit; ++pos) {
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 2. main loop.
  limit = size - radius;
  // SSE4.1 solution
  const uint8_t *subptr = input.data() + pos - radius - 1;
  const uint8_t *addptr = input.data() + pos + radius;
  const uint16_t *outptr = output.data() + pos;
  __m128i current = _mm_set1_epi16(currentSum);
  int i = 0;
  // loop processes 8 elements per iteration
  // original scalar condition:
  //     ++pos < limit
  //
  // now we use `pos + i` instead of `pos`, so the new condition becomes:
  //     pos + i < limit
  //
  // but since we process 8 elements per iteration, we must ensure the last element stays within bounds:
  //     pos + i + 7 < limit
  //     => i + 7 < limit - pos
  for (; i + 7 < limit - pos; i += 8) {
    // 1. Calculate vector diff: input[i+radius] - input[i-radius-1]
    __m128i sub8 = _mm_loadu_si64(subptr + i);
    __m128i add8 = _mm_loadu_si64(addptr + i);
    __m128i sub16 = _mm_cvtepu8_epi16(sub8);
    __m128i add16 = _mm_cvtepu8_epi16(add8);
    __m128i diffs = _mm_sub_epi16(add16, sub16);

    // 2. Calculate vector prefix sum for 8 elements
    __m128i prefix_diffs = _mm_add_epi16(diffs, _mm_slli_si128(diffs, 2));
    prefix_diffs = _mm_add_epi16(prefix_diffs, _mm_slli_si128(prefix_diffs, 4));
    prefix_diffs = _mm_add_epi16(prefix_diffs, _mm_slli_si128(prefix_diffs, 8));
    
    // 3. Store the result
    __m128i out = _mm_add_epi16(current, prefix_diffs);
    _mm_storeu_si128((__m128i *)(outptr + i), out);
    
    // 4. Broadcast currentSum for the next iteration
    currentSum = (uint16_t)_mm_extract_epi16(out, 0b111);
    current = _mm_set1_epi16(currentSum);
  }
  pos += i;

  // Still keep the sequential loop to process the remainder.
  for (; pos < limit; ++pos) {
    currentSum -= input[pos - radius - 1];
    currentSum += input[pos + radius];
    output[pos] = currentSum;
  }

  // 3. special case, executed only if size <= 2*radius + 1
  limit = std::min(radius + 1, size);
  for (; pos < limit; pos++) {
    output[pos] = currentSum;
  }

  // 4. right border - time spend in this loop can be ignored, no need to
  // optimize it
  for (; pos < size; ++pos) {
    currentSum -= input[pos - radius - 1];
    output[pos] = currentSum;
  }
}
