/* slice.c
   authors: Simon Lovell Bart (microchips-n-dip)
            David Robinson Garland (davidgarland)
   description: A set of slice utilities.
   date: 2019-06-04
   */

/* Dependencies */

#include <chaos.h>

/* Helper Functions */

struct slice
slice_combine(struct slice a, struct slice b) {
  assert(a.left <= a.right);
  assert(b.left <= b.right);

  struct slice s;
  s.left  = (a.left  < b.left)  ? a.left  : b.left;
  s.right = (a.right > b.right) ? a.right : b.right;
  return s;
}

/* Unit Slice Functions */

unsigned long long
unit_slice_atoi(struct unit *unit, struct slice s) {
  assert(unit != NULL);
  assert(unit->src != NULL);
  assert(s.left <= s.right);
   
  unsigned long long result = 0;

  for (size_t i = s.left; i <= s.right; i++)
    result = ((result << 3) + (result << 2)) + (unit->src[i] - '0');

  return result;
}

int
unit_slice_cmp(struct unit *unit, struct slice a, struct slice b) {
  assert(unit != NULL);
  assert(unit->src != NULL);
  assert(a.left <= a.right);
  assert(b.left <= b.right);

  size_t len_a = a.right - a.left + 1;
  size_t len_b = b.right - b.left + 1;
  
  if (likely(len_a != len_b))
    return 0;
  
  for (size_t i = 0; i < len_a; i++)
    if (likely(unit->src[a.left + i] != unit->src[b.left + i]))
      return 0;
  
  return 1;
}

int
unit_slice_cmp_str(struct unit *unit, struct slice s, char *c) {
  assert(unit != NULL);
  assert(unit->src != NULL);
  assert(s.left <= s.right);
  assert(c != NULL);

  size_t len_s = s.right - s.left + 1;
  size_t len_c = strlen(c);

  if (likely(len_s != len_c))
    return 0;

  for (size_t i = 0; i < len_s; i++)
    if (likely(unit->src[s.left + i] != c[i]))
      return 0;
  
  return 1;
}
