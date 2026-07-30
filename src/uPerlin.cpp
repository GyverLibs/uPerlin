// SPDX-License-Identifier: MIT
// Integer Perlin noise adapted from FastLED.
// See LICENSE and THIRD_PARTY_NOTICES.md.

#include "uPerlin.h"

#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

namespace {

#if defined(__AVR__)
#define UPERLIN_PROGMEM PROGMEM
#else
#define UPERLIN_PROGMEM
#endif

// Ken Perlin's permutation table plus the first element at index 256.
static const uint8_t permutation[257] UPERLIN_PROGMEM = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151};

static inline uint8_t perm(uint16_t index) {
#if defined(__AVR__)
    return pgm_read_byte_near(permutation + index);
#else
    return permutation[index];
#endif
}

// -----------------------------------------------------------------------------
// Shared lattice hashing

struct Hash1 {
    uint8_t a;
    uint8_t b;
};

struct Hash2 {
    uint8_t aa;
    uint8_t ab;
    uint8_t ba;
    uint8_t bb;
};

struct Hash3 {
    uint8_t aa;
    uint8_t ab;
    uint8_t ba;
    uint8_t bb;
};

static inline Hash1 hash1(uint8_t x) {
    Hash1 h;
    uint8_t a = perm(x);
    uint8_t b = perm((uint16_t)x + 1U);
    h.a = perm(perm(a));
    h.b = perm(perm(b));
    return h;
}

static inline Hash2 hash2(uint8_t x, uint8_t y) {
    Hash2 h;
    uint8_t a = (uint8_t)(perm(x) + y);
    uint8_t b = (uint8_t)(perm((uint16_t)x + 1U) + y);
    h.aa = perm(perm(a));
    h.ab = perm(perm((uint16_t)a + 1U));
    h.ba = perm(perm(b));
    h.bb = perm(perm((uint16_t)b + 1U));
    return h;
}

static inline Hash3 hash3(uint8_t x, uint8_t y, uint8_t z) {
    Hash3 h;
    uint8_t a = (uint8_t)(perm(x) + y);
    uint8_t b = (uint8_t)(perm((uint16_t)x + 1U) + y);
    h.aa = (uint8_t)(perm(a) + z);
    h.ab = (uint8_t)(perm((uint16_t)a + 1U) + z);
    h.ba = (uint8_t)(perm(b) + z);
    h.bb = (uint8_t)(perm((uint16_t)b + 1U) + z);
    return h;
}

// -----------------------------------------------------------------------------
// 8-bit fixed-point math

static inline int8_t wrap8(int16_t value) {
    uint8_t u = (uint8_t)value;
    return u <= 0x7FU ? (int8_t)u : (int8_t)((int16_t)u - 256);
}

static inline int8_t negate8(int8_t value) {
    return wrap8((int16_t)(-(int16_t)value));
}

static inline int16_t floorHalf8(int16_t value) {
    return value >= 0 ? (int16_t)(value / 2)
                      : (int16_t)(-(((-value) + 1) / 2));
}

static inline int8_t average8(int8_t a, int8_t b) {
    int16_t ah = floorHalf8((int16_t)a);
    int16_t bh = floorHalf8((int16_t)b);
    int16_t lsb = (int16_t)a - (int16_t)(2 * ah);
    return wrap8((int16_t)(ah + bh + lsb));
}

static inline uint8_t scale8(uint8_t value, uint8_t amount) {
    return (uint8_t)(((uint16_t)value * ((uint16_t)amount + 1U)) >> 8);
}

// FastLED-compatible quadratic easing.  It is cheaper than cubic smoothstep
// and keeps the familiar inoise shape.
static inline uint8_t ease8(uint8_t value) {
    uint8_t mirrored = value;
    uint8_t squared;

    if ((value & 0x80U) != 0U) mirrored = (uint8_t)(255U - value);
    squared = (uint8_t)(scale8(mirrored, mirrored) << 1);
    return (value & 0x80U) != 0U ? (uint8_t)(255U - squared) : squared;
}

static inline int8_t lerp8(int8_t a, int8_t b, uint8_t fraction) {
    uint8_t delta;
    if (b > a) {
        delta = (uint8_t)((int16_t)b - (int16_t)a);
        return wrap8((int16_t)a + (int16_t)scale8(delta, fraction));
    }

    delta = (uint8_t)((int16_t)a - (int16_t)b);
    return wrap8((int16_t)a - (int16_t)scale8(delta, fraction));
}

// FastLED-like 1D gradient set.  The original branch can average +x and -x
// to zero; that is the sole source of completely flat lattice cells.  In
// that one case use a half-strength gradient instead, preserving the varied
// FastLED shape without adding a multiply or lookup table.
static inline int8_t grad1_8(uint8_t hash, int8_t x) {
    int8_t u;
    int8_t v;

    if ((hash & 8U) != 0U) {
        if (((hash ^ (hash >> 1)) & 1U) != 0U) {
            int8_t half = average8(x, 0);
            return (hash & 1U) != 0U ? negate8(half) : half;
        }
        return (hash & 1U) != 0U ? negate8(x) : x;
    }

    if ((hash & 4U) != 0U) {
        u = 1;
        v = x;
    } else {
        u = x;
        v = 1;
    }

    if ((hash & 1U) != 0U) u = negate8(u);
    if ((hash & 2U) != 0U) v = negate8(v);
    return average8(u, v);
}

static inline int8_t grad2_8(uint8_t hash, int8_t x, int8_t y) {
    int8_t u = (hash & 4U) != 0U ? y : x;
    int8_t v = (hash & 4U) != 0U ? x : y;
    if ((hash & 1U) != 0U) u = negate8(u);
    if ((hash & 2U) != 0U) v = negate8(v);
    return average8(u, v);
}

static inline int8_t grad3_8(uint8_t hash, int8_t x, int8_t y, int8_t z) {
    switch (hash & 0x0FU) {
        case 0: return average8(x, y);
        case 1: return average8(negate8(x), y);
        case 2: return average8(x, negate8(y));
        case 3: return average8(negate8(x), negate8(y));
        case 4: return average8(x, z);
        case 5: return average8(negate8(x), z);
        case 6: return average8(x, negate8(z));
        case 7: return average8(negate8(x), negate8(z));
        case 8: return average8(y, z);
        case 9: return average8(negate8(y), z);
        case 10: return average8(y, negate8(z));
        case 11: return average8(negate8(y), negate8(z));
        case 12: return average8(x, y);
        case 13: return average8(negate8(x), y);
        case 14: return average8(x, negate8(y));
        default: return average8(negate8(x), negate8(y));
    }
}

// -----------------------------------------------------------------------------
// 16-bit fixed-point math

static inline int16_t wrap16(int32_t value) {
    uint16_t u = (uint16_t)value;
    return u <= 0x7FFFU ? (int16_t)u : (int16_t)((int32_t)u - 65536L);
}

static inline int16_t negate16(int16_t value) {
    return wrap16(-(int32_t)value);
}

static inline int32_t floorHalf16(int32_t value) {
    return value >= 0 ? value / 2 : -(((-value) + 1) / 2);
}

static inline int16_t average16(int16_t a, int16_t b) {
    int32_t ah = floorHalf16((int32_t)a);
    int32_t bh = floorHalf16((int32_t)b);
    int32_t lsb = (int32_t)a - 2 * ah;
    return wrap16(ah + bh + lsb);
}

static inline uint16_t scale16(uint16_t value, uint16_t amount) {
    return (uint16_t)(((uint32_t)value * ((uint32_t)amount + 1UL)) >> 16);
}

// FastLED-compatible quadratic easing.
static inline uint16_t ease16(uint16_t value) {
    uint16_t mirrored = value;
    uint16_t squared;

    if ((value & 0x8000U) != 0U) mirrored = (uint16_t)(65535UL - value);
    squared = (uint16_t)(scale16(mirrored, mirrored) << 1);
    return (value & 0x8000U) != 0U
               ? (uint16_t)(65535UL - squared)
               : squared;
}

static inline int16_t lerp16(int16_t a, int16_t b, uint16_t fraction) {
    uint16_t delta;
    if (b > a) {
        delta = (uint16_t)((int32_t)b - (int32_t)a);
        return wrap16((int32_t)a + (int32_t)scale16(delta, fraction));
    }

    delta = (uint16_t)((int32_t)a - (int32_t)b);
    return wrap16((int32_t)a - (int32_t)scale16(delta, fraction));
}

static inline int16_t grad1_16(uint8_t hash, int16_t x) {
    int16_t u;
    int16_t v;

    if ((hash & 8U) != 0U) {
        if (((hash ^ (hash >> 1)) & 1U) != 0U) {
            int16_t half = average16(x, 0);
            return (hash & 1U) != 0U ? negate16(half) : half;
        }
        return (hash & 1U) != 0U ? negate16(x) : x;
    }

    if ((hash & 4U) != 0U) {
        u = 1;
        v = x;
    } else {
        u = x;
        v = 1;
    }

    if ((hash & 1U) != 0U) u = negate16(u);
    if ((hash & 2U) != 0U) v = negate16(v);
    return average16(u, v);
}

static inline int16_t grad2_16(uint8_t hash, int16_t x, int16_t y) {
    int16_t u = (hash & 4U) != 0U ? y : x;
    int16_t v = (hash & 4U) != 0U ? x : y;
    if ((hash & 1U) != 0U) u = negate16(u);
    if ((hash & 2U) != 0U) v = negate16(v);
    return average16(u, v);
}

static inline int16_t grad3_16(uint8_t hash, int16_t x, int16_t y, int16_t z) {
    int16_t u;
    int16_t v;

    hash &= 15U;
    u = hash < 8U ? x : y;
    v = hash < 4U ? y : ((hash == 12U || hash == 14U) ? x : z);
    if ((hash & 1U) != 0U) u = negate16(u);
    if ((hash & 2U) != 0U) v = negate16(v);
    return average16(u, v);
}

// -----------------------------------------------------------------------------
// Output scaling

static inline uint8_t scaleOutput8(int8_t raw) {
    int16_t shifted = (int16_t)raw + 64;
    uint16_t doubled;

    if (shifted <= 0) return 0;
    doubled = (uint16_t)shifted * 2U;
    return doubled > 255U ? 255U : (uint8_t)doubled;
}

static inline uint16_t scaleOutput16_1d(int16_t raw) {
    int32_t shifted = (int32_t)raw + 17308L;
    uint32_t doubled;

    if (shifted <= 0) return 0;
    doubled = (uint32_t)shifted * 2UL;
    return doubled > 65535UL ? 65535U : (uint16_t)doubled;
}

static inline uint16_t scaleOutput16_2d(int16_t raw) {
    int32_t shifted = (int32_t)raw + 17308L;
    return (uint16_t)(((uint32_t)shifted * 484UL) >> 8);
}

static inline uint16_t scaleOutput16_3d(int16_t raw) {
    int32_t shifted = (int32_t)raw + 19052L;
    return (uint16_t)(((uint32_t)shifted * 440UL) >> 8);
}

}  // anonymous namespace

// -----------------------------------------------------------------------------
// Public 8-bit API

int8_t uperlin8Raw(uint16_t x) {
    uint8_t cellX = (uint8_t)(x >> 8);
    uint8_t u = (uint8_t)x;
    int8_t xx = (int8_t)((u >> 1) & 0x7FU);
    int8_t xx1 = (int8_t)((int16_t)xx - 128);
    Hash1 h = hash1(cellX);

    u = ease8(u);
    return lerp8(grad1_8(h.a, xx), grad1_8(h.b, xx1), u);
}

int8_t uperlin8Raw(uint16_t x, uint16_t y) {
    uint8_t u = (uint8_t)x;
    uint8_t v = (uint8_t)y;
    int8_t xx = (int8_t)((u >> 1) & 0x7FU);
    int8_t yy = (int8_t)((v >> 1) & 0x7FU);
    int8_t xx1 = (int8_t)((int16_t)xx - 128);
    int8_t yy1 = (int8_t)((int16_t)yy - 128);
    int8_t a;
    int8_t b;
    Hash2 h = hash2((uint8_t)(x >> 8), (uint8_t)(y >> 8));

    u = ease8(u);
    v = ease8(v);
    a = lerp8(grad2_8(h.aa, xx, yy), grad2_8(h.ba, xx1, yy), u);
    b = lerp8(grad2_8(h.ab, xx, yy1), grad2_8(h.bb, xx1, yy1), u);
    return lerp8(a, b, v);
}

int8_t uperlin8Raw(uint16_t x, uint16_t y, uint16_t z) {
    uint8_t u = (uint8_t)x;
    uint8_t v = (uint8_t)y;
    uint8_t w = (uint8_t)z;
    int8_t xx = (int8_t)((u >> 1) & 0x7FU);
    int8_t yy = (int8_t)((v >> 1) & 0x7FU);
    int8_t zz = (int8_t)((w >> 1) & 0x7FU);
    int8_t xx1 = (int8_t)((int16_t)xx - 128);
    int8_t yy1 = (int8_t)((int16_t)yy - 128);
    int8_t zz1 = (int8_t)((int16_t)zz - 128);
    int8_t a;
    int8_t b;
    int8_t c;
    int8_t d;
    Hash3 h = hash3((uint8_t)(x >> 8), (uint8_t)(y >> 8), (uint8_t)(z >> 8));

    u = ease8(u);
    v = ease8(v);
    w = ease8(w);

    a = lerp8(grad3_8(perm(h.aa), xx, yy, zz),
              grad3_8(perm(h.ba), xx1, yy, zz), u);
    b = lerp8(grad3_8(perm(h.ab), xx, yy1, zz),
              grad3_8(perm(h.bb), xx1, yy1, zz), u);
    c = lerp8(grad3_8(perm((uint16_t)h.aa + 1U), xx, yy, zz1),
              grad3_8(perm((uint16_t)h.ba + 1U), xx1, yy, zz1), u);
    d = lerp8(grad3_8(perm((uint16_t)h.ab + 1U), xx, yy1, zz1),
              grad3_8(perm((uint16_t)h.bb + 1U), xx1, yy1, zz1), u);

    return lerp8(lerp8(a, b, v), lerp8(c, d, v), w);
}

uint8_t uperlin8(uint16_t x) {
    return scaleOutput8(uperlin8Raw(x));
}

uint8_t uperlin8(uint16_t x, uint16_t y) {
    return scaleOutput8(uperlin8Raw(x, y));
}

uint8_t uperlin8(uint16_t x, uint16_t y, uint16_t z) {
    return scaleOutput8(uperlin8Raw(x, y, z));
}

// -----------------------------------------------------------------------------
// Public 16-bit API

int16_t uperlin16Raw(uint32_t x) {
    uint8_t cellX = (uint8_t)(x >> 16);
    uint16_t u = (uint16_t)x;
    int16_t xx = (int16_t)((u >> 1) & 0x7FFFU);
    int16_t xx1 = (int16_t)((int32_t)xx - 32768L);
    Hash1 h = hash1(cellX);

    u = ease16(u);
    return lerp16(grad1_16(h.a, xx), grad1_16(h.b, xx1), u);
}

int16_t uperlin16Raw(uint32_t x, uint32_t y) {
    uint16_t u = (uint16_t)x;
    uint16_t v = (uint16_t)y;
    int16_t xx = (int16_t)((u >> 1) & 0x7FFFU);
    int16_t yy = (int16_t)((v >> 1) & 0x7FFFU);
    int16_t xx1 = (int16_t)((int32_t)xx - 32768L);
    int16_t yy1 = (int16_t)((int32_t)yy - 32768L);
    int16_t a;
    int16_t b;
    Hash2 h = hash2((uint8_t)(x >> 16), (uint8_t)(y >> 16));

    u = ease16(u);
    v = ease16(v);
    a = lerp16(grad2_16(h.aa, xx, yy), grad2_16(h.ba, xx1, yy), u);
    b = lerp16(grad2_16(h.ab, xx, yy1), grad2_16(h.bb, xx1, yy1), u);
    return lerp16(a, b, v);
}

int16_t uperlin16Raw(uint32_t x, uint32_t y, uint32_t z) {
    uint16_t u = (uint16_t)x;
    uint16_t v = (uint16_t)y;
    uint16_t w = (uint16_t)z;
    int16_t xx = (int16_t)((u >> 1) & 0x7FFFU);
    int16_t yy = (int16_t)((v >> 1) & 0x7FFFU);
    int16_t zz = (int16_t)((w >> 1) & 0x7FFFU);
    int16_t xx1 = (int16_t)((int32_t)xx - 32768L);
    int16_t yy1 = (int16_t)((int32_t)yy - 32768L);
    int16_t zz1 = (int16_t)((int32_t)zz - 32768L);
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    Hash3 h = hash3((uint8_t)(x >> 16), (uint8_t)(y >> 16), (uint8_t)(z >> 16));

    u = ease16(u);
    v = ease16(v);
    w = ease16(w);

    a = lerp16(grad3_16(perm(h.aa), xx, yy, zz),
               grad3_16(perm(h.ba), xx1, yy, zz), u);
    b = lerp16(grad3_16(perm(h.ab), xx, yy1, zz),
               grad3_16(perm(h.bb), xx1, yy1, zz), u);
    c = lerp16(grad3_16(perm((uint16_t)h.aa + 1U), xx, yy, zz1),
               grad3_16(perm((uint16_t)h.ba + 1U), xx1, yy, zz1), u);
    d = lerp16(grad3_16(perm((uint16_t)h.ab + 1U), xx, yy1, zz1),
               grad3_16(perm((uint16_t)h.bb + 1U), xx1, yy1, zz1), u);

    return lerp16(lerp16(a, b, v), lerp16(c, d, v), w);
}

uint16_t uperlin16(uint32_t x) {
    return scaleOutput16_1d(uperlin16Raw(x));
}

uint16_t uperlin16(uint32_t x, uint32_t y) {
    return scaleOutput16_2d(uperlin16Raw(x, y));
}

uint16_t uperlin16(uint32_t x, uint32_t y, uint32_t z) {
    return scaleOutput16_3d(uperlin16Raw(x, y, z));
}
