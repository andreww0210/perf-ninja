#include "solution.hpp"
#include <cstdint>

uint16_t checksum(const Blob &blob) {
  uint32_t acc = 0;
  for (auto value : blob)
    acc += value;

  uint16_t high = acc >> 16;
  // uint16_t low = (uint16_t)acc;
  uint16_t low = acc & 0xFFFFu;
  low += high;
  // account for potential overflow.
  low += low < high;
  return low;
}
