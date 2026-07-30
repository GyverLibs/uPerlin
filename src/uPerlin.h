// SPDX-License-Identifier: MIT
// Integer Perlin noise adapted from FastLED.
// See LICENSE and THIRD_PARTY_NOTICES.md.

#ifndef UPERLIN_H
#define UPERLIN_H

#include <stdint.h>

// 8-bit output, integer coordinates
uint8_t uperlin8(uint16_t x);
uint8_t uperlin8(uint16_t x, uint16_t y);
uint8_t uperlin8(uint16_t x, uint16_t y, uint16_t z);

// 16-bit output, integer coordinates
uint16_t uperlin16(uint32_t x);
uint16_t uperlin16(uint32_t x, uint32_t y);
uint16_t uperlin16(uint32_t x, uint32_t y, uint32_t z);

// 8-bit output, Q8.8 coordinates
int8_t uperlin8Raw(uint16_t x);
int8_t uperlin8Raw(uint16_t x, uint16_t y);
int8_t uperlin8Raw(uint16_t x, uint16_t y, uint16_t z);

// 16-bit output, Q16.16 coordinates
int16_t uperlin16Raw(uint32_t x);
int16_t uperlin16Raw(uint32_t x, uint32_t y);
int16_t uperlin16Raw(uint32_t x, uint32_t y, uint32_t z);

#endif
