/* alias.h
   author: Simon Lovell Bart (microchips-n-dip)
   description: A small collection of macros I find useful for making code
                neater.
   date: 2018-05-31
   */

#ifndef ALIAS_H
#define ALIAS_H

/* Rename the `__builtin_expect()`s. */

#define unlikely(x) __builtin_expect((x), 0)
#define likely(x)   __builtin_expect(!(x), 0)

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

#endif /* ALIAS_H */

