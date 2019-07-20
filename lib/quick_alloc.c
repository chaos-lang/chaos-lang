#include <stdlib.h>

#include "quick_alloc.h"

struct qa_chunk {
  struct qa_chunk *prev, *next;
  unsigned long used;
  char data[1];
};

#define QA_CHUNK_ALLOC_SIZE 2056
#define QA_CHUNK_ALLOC_HEAD (sizeof(struct qa_chunk) - 1)
#define QA_CHUNK_ALLOC_BODY (QA_CHUNK_ALLOC_SIZE - QA_CHUNK_ALLOC_HEAD)

/* Allocate a new `qa_chunk`. If `*chunk` is not NULL, append the new chunk. */

static void
new_qa_chunk(struct qa_chunk **chunk, unsigned long size) {
  struct qa_chunk *new_chunk;
  if (size > QA_CHUNK_ALLOC_BODY)
    new_chunk = malloc(2 * size + QA_CHUNK_ALLOC_HEAD);
  else
    new_chunk = malloc(QA_CHUNK_ALLOC_SIZE);
  new_chunk->prev = 0;
  new_chunk->next = 0;
  new_chunk->used = 0;
  if (*chunk) {
    (*chunk)->next = new_chunk;
    new_chunk->prev = *chunk;
  }
  *chunk = new_chunk;
}

static struct qa_chunk *cur_chunk;

void
quick_alloc_init(void) {
  new_qa_chunk(&cur_chunk, 0);
}

/* Allocate from a quick alloc chunk. For very quick, small, easy
   allocations. */

void *
quick_alloc(unsigned long size) {
  if (cur_chunk->used + size >= QA_CHUNK_ALLOC_BODY)
    new_qa_chunk(&cur_chunk, size);
  void *p = (void *) (cur_chunk->data + cur_chunk->used);
  cur_chunk->used += size;
  return p;
}

/* Free all quick alloc chunks. REQUIRES REINITIALIZATION. */

void
qa_free_all(void) {
  struct qa_chunk *chunk;
  while (cur_chunk) {
    chunk = cur_chunk;
    cur_chunk = chunk->prev;
    free(chunk);
  }
}
