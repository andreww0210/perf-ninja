#include "solution.hpp"
#include <algorithm>
#include <cstdint>
#include <immintrin.h>
#include <smmintrin.h>
#include <vector>

Position<std::uint32_t> solution(std::vector<Position<std::uint32_t>> const &input) {
  std::uint64_t x = 0;
  std::uint64_t y = 0;
  std::uint64_t z = 0;

  #ifdef ORIGIN
  // Origin version:
  // This can be vectorized on new x86 core by using `vpermt2d`.
  for (auto pos: input) {
    x += pos.x;
    y += pos.y;
    z += pos.z;
  }
  #endif

  #ifdef SOA
  // AoS -> SoA version. But not good.
  struct SoA {
    std::vector<std::uint32_t> vector_x;
    std::vector<std::uint32_t> vector_y;
    std::vector<std::uint32_t> vector_z;
  } soa;

  const size_t n = input.size();
  soa.vector_x.reserve(n);
  soa.vector_y.reserve(n);
  soa.vector_z.reserve(n);
  
  // This introduce a new bottle-neck.
  for (auto pos : input) {
    soa.vector_x.push_back(pos.x);
    soa.vector_y.push_back(pos.y);
    soa.vector_z.push_back(pos.z);
  }

  // This is faster.
  for (int i = 0; i < soa.vector_x.size(); i++) {
    x += soa.vector_x[i];
    y += soa.vector_y[i];
    z += soa.vector_z[i];
  }
  #endif

  #ifdef MY_SIMD
  // I wrote a SIMD version myself.
  // It's not very good, but the logic is correct.
  int i = 0;
  uint64_t acc_x = 0, acc_y = 0, acc_z = 0;
  const struct Position<std::uint32_t> *src = input.data();
  __m512i idx_x =
      _mm512_set_epi32(12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0);
  __m512i idx_y =
      _mm512_set_epi32(13, 9, 5, 1, 13, 9, 5, 1, 13, 9, 5, 1, 13, 9, 5, 1);
  __m512i idx_z =
      _mm512_set_epi32(14, 10, 6, 2, 14, 10, 6, 2, 14, 10, 6, 2, 14, 10, 6, 2);

  for (; i + 4 < input.size(); i += 4) {
    // load [16 * i32]
    __m512i data_vec = _mm512_loadu_si512((const __m512i *)&src[i]);
    // 16 * input[i].x, only [0:3] is valid.
    __m512i vector_x = _mm512_permutexvar_epi32(idx_x, data_vec);
    // 16 * input[i].y, only [0:3] is valid.
    __m512i vector_y = _mm512_permutexvar_epi32(idx_y, data_vec);
    // 16 * input[i].z, only [0:3] is valid.
    __m512i vector_z = _mm512_permutexvar_epi32(idx_z, data_vec);

    __m256i vec_x_lo = _mm512_extracti32x8_epi32(vector_x, 0);
    __m256i vec_y_lo = _mm512_extracti32x8_epi32(vector_y, 0);
    __m256i vec_z_lo = _mm512_extracti32x8_epi32(vector_z, 0);

    __m512i vec_x = _mm512_cvtepu32_epi64(vec_x_lo);
    __m512i vec_y = _mm512_cvtepu32_epi64(vec_y_lo);
    __m512i vec_z = _mm512_cvtepu32_epi64(vec_z_lo);

    acc_x += _mm512_reduce_add_epi32(vec_x) / 2;
    acc_y += _mm512_reduce_add_epi32(vec_y) / 2;
    acc_z += _mm512_reduce_add_epi32(vec_z) / 2;
  }

  for (; i < input.size(); i++) {
    acc_x += input[i].x;
    acc_y += input[i].y;
    acc_z += input[i].z;
  }

  x = acc_x;
  y = acc_y;
  z = acc_z;
  #endif

  int i = 0;
  const __m128i *intputStr = reinterpret_cast<const __m128i *>(&input[0].x);
  __m256i sum_xyzx = _mm256_setzero_si256();
  __m256i sum_yzxy = _mm256_setzero_si256();
  __m256i sum_zxyz = _mm256_setzero_si256();

  for (; i + 3 < input.size(); i += 4) {
    __m128i XYZX = _mm_load_epi32(intputStr);
    __m128i YZXY =_mm_load_epi32(intputStr + 1);
    __m128i ZXYZ =_mm_load_epi32(intputStr + 2);

    __m256i XYZX_64 = _mm256_cvtepu32_epi64(XYZX);
    __m256i YZXY_64 = _mm256_cvtepu32_epi64(YZXY);
    __m256i ZXYZ_64 = _mm256_cvtepu32_epi64(ZXYZ);

    sum_xyzx = _mm256_add_epi64(sum_xyzx, XYZX_64);
    sum_yzxy = _mm256_add_epi64(sum_yzxy, YZXY_64);
    sum_zxyz = _mm256_add_epi64(sum_zxyz, ZXYZ_64);

    intputStr += 3;
  }

  // [Denis]: I implemented this naive reduction.
  // Perhaps you can do it better with vpermq or similar, but I was lazy.
  // But does it really matter? I guess you would see no difference
  // since this code not in the hot loop.
  x += _mm256_extract_epi64(sum_xyzx, 0b00);
  y += _mm256_extract_epi64(sum_xyzx, 0b01);
  z += _mm256_extract_epi64(sum_xyzx, 0b10);
  x += _mm256_extract_epi64(sum_xyzx, 0b11);

  y += _mm256_extract_epi64(sum_yzxy, 0b00);
  z += _mm256_extract_epi64(sum_yzxy, 0b01);
  x += _mm256_extract_epi64(sum_yzxy, 0b10);
  y += _mm256_extract_epi64(sum_yzxy, 0b11);

  z += _mm256_extract_epi64(sum_zxyz, 0b00);
  x += _mm256_extract_epi64(sum_zxyz, 0b01);
  y += _mm256_extract_epi64(sum_zxyz, 0b10);
  z += _mm256_extract_epi64(sum_zxyz, 0b11);

  for (; i < input.size(); i++) {
    x += input[i].x;
    y += input[i].y;
    z += input[i].z;
  }

  return {
          static_cast<std::uint32_t>(x / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(y / std::max<std::uint64_t>(1, input.size())),
          static_cast<std::uint32_t>(z / std::max<std::uint64_t>(1, input.size())),
  };
}