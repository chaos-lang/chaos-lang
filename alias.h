/* alias.h
   author: Simon Lovell Bart (microchips-n-dip)
   description: A small collection of macros I find useful for making code
                neater.
   date: 2018-05-31
   */

#ifndef ALIAS_H
#define ALIAS_H

/* Rename the `__builtin_expect()`s if available. */

#ifndef unlikely
#ifdef __GNUC__
#define unlikely(x) __builtin_expect((x), 0)
#else
#define unlikely(x) (x)
#endif /* __GNUC__ */
#endif /* unlikely */

#ifndef likely
#ifdef __GNUC__
#define likely(x)   __builtin_expect(!!(x), !0)
#else
#define likely(x)   (x)
#endif /* __GNUC__ */
#endif /* likely */

/* Make our own offsetof macro if necessary. */

#ifndef offsetof
#ifdef __GNUC__
#define offsetof(TYPE, M) __builtin_offsetof(TYPE, M)
#else
#define offsetof(TYPE, M) ((unsigned long) &((TYPE *) 0)->M)
#endif /* __GNUC__ */
#endif /* offsetof */

/* Allocator macros. */

#define XNEW(T)             (T *) malloc(sizeof(T))
#define XCNEW(T)            (T *) calloc(1, sizeof(T))
#define XDELETE(P)          free((void *) (P))

#define XNEWVEC(T, N)       (T *) malloc(sizeof(T) * (N))
#define XCNEWVEC(T, N)      (T *) calloc((N), sizeof(T))
#define XRESIZEVEC(T, P, N) (T *) realloc((void *) (P), sizeof(T) * (N))
#define XDELETEVEC(P)       free((void *) (P))

/* Some macros that will be useful for timing code. */

#define timing_start(tp0)                 \
  do {                                    \
    clock_gettime(CLOCK_MONOTONIC, &tp0); \
  } while (0)

#define timing_stop(str, tp0, tp1)        \
  do {                                    \
    clock_gettime(CLOCK_MONOTONIC, &tp1); \
    printf("%s took %ld ticks\n", str,    \
      tp1.tv_nsec - tp0.tv_nsec);         \
  } while (0);

#endif /* ALIAS_H */

