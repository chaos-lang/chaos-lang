/* xxhash.c
   author: Simon Lovell Bart (microchips-n-dip)
   description: xxHash implementation for Cheshire.
                (see https://github.com/Cyan4973/xxHash/blob/dev/doc/xxhash_spec.md)
   date: 2019-07-19
   */

#include <stdlib.h>
#include <string.h>

#include "xxhash.h"

/* TODO: Comment, add compiler-specific requirements, optimize. */

typedef unsigned char  BYTE;
typedef unsigned short U16;
typedef unsigned int   U32;

#ifndef CPU_LITTLE_ENDIAN
static int
little_endian(void) {
  const union {
    U32 u;
    BYTE c[4];
  } one = {1};
  return one.c[0];
}
#define CPU_LITTLE_ENDIAN little_endian()
#endif

#define FORCE_INLINE static inline __attribute__((always_inline))

#define rotl32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))
#define rotl64(x, r) (((x) << (r)) | ((x) >> (64 - (r))))

static U32
swap32(U32 x) {
  return ((x << 24) & 0xff000000) |
         ((x <<  8) & 0x00ff0000) |
         ((x >>  8) & 0x0000ff00) |
         ((x >> 24) & 0x000000ff);
}

enum alignment {
  aligned,
  unaligned
};

static U32
read32(const void *p) {
  U32 val;
  memcpy(&val, p, sizeof(val));
  return val;
}

FORCE_INLINE U32
read_LE32(const void *ptr) {
  return CPU_LITTLE_ENDIAN ? read32(ptr) : swap32(read32(ptr));
}

FORCE_INLINE U32
read_LE32_align(const void *ptr, enum alignment align) {
  if (align == unaligned)
    return read_LE32(ptr);
  else
    return CPU_LITTLE_ENDIAN ? *(const U32 *) ptr : swap32(*(const U32 *) ptr);
}

#define get32bits(p) read_LE32_align(p, align)

static const U32 PRIME32_1 = 2654435761U;
static const U32 PRIME32_2 = 2246822519U;
static const U32 PRIME32_3 = 3266489917U;
static const U32 PRIME32_4 =  668265263U;
static const U32 PRIME32_5 =  374761393U;

static U32
round32(U32 acc, U32 input) {
  acc += input * PRIME32_2;
  acc = rotl32(acc, 13);
  acc *= PRIME32_1;
  __asm__("" : "+r" (acc));
  return acc;
}

static U32
avalanche32(U32 h32) {
  h32 ^= h32 >> 15;
  h32 *= PRIME32_2;
  h32 ^= h32 >> 13;
  h32 *= PRIME32_3;
  h32 ^= h32 >> 16;
  return h32;
}

static U32
finalize32(U32 h32, const void *ptr, size_t len, enum alignment align) {
  const BYTE *p = (const BYTE *) ptr;
#define PROCESS1                       \
  do {                                 \
    h32 += (*p++) * PRIME32_5;         \
    h32 = rotl32(h32, 11) * PRIME32_1; \
  } while (0)
#define PROCESS4                       \
  do {                                 \
    h32 += get32bits(p) * PRIME32_3;   \
    p += 4;                            \
    h32 = rotl32(h32, 17) * PRIME32_4; \
  } while (0)
  switch (len & 15) {
    case 12: PROCESS4;
    case  8: PROCESS4;
    case  4: PROCESS4;
             return avalanche32(h32);
    case 13: PROCESS4;
    case  9: PROCESS4;
    case  5: PROCESS4;
             PROCESS1;
             return avalanche32(h32);
    case 14: PROCESS4;
    case 10: PROCESS4;
    case  6: PROCESS4;
             PROCESS1;
             PROCESS1;
             return avalanche32(h32);
    case 15: PROCESS4;
    case 11: PROCESS4;
    case  7: PROCESS4;
    case  3: PROCESS1;
    case  2: PROCESS1;
    case  1: PROCESS1;
    case  0: return avalanche32(h32);
  }
}

FORCE_INLINE U32
endian_align_32(const void *input, size_t len, U32 seed,
    enum alignment align) {
  const BYTE *p = (const BYTE *) input;
  const BYTE *b_end = p + len;
  U32 h32;
  if (len >= 16) {
    const BYTE *const limit = b_end - 15;
    U32 v1 = seed + PRIME32_1 + PRIME32_2;
    U32 v2 = seed + PRIME32_2;
    U32 v3 = seed + 0;
    U32 v4 = seed - PRIME32_1;
    do {
      v1 = round32(v1, get32bits(p)); p += 4;
      v2 = round32(v2, get32bits(p)); p += 4;
      v3 = round32(v3, get32bits(p)); p += 4;
      v4 = round32(v4, get32bits(p)); p += 4;
    } while (p < limit);
    h32 = rotl32(v1, 1) + rotl32(v2, 7) + rotl32(v3, 12) + rotl32(v4, 18);
  }
  else {
    h32 = seed + PRIME32_5;
  }
  h32 += (U32) len;
  return finalize32(h32, p, len & 15, align);
}

unsigned int
xxhash32(const void *input, size_t len, unsigned int seed) {
  return endian_align_32(input, len, seed, unaligned);
}

