#ifndef QUICK_ALLOC_H
#define QUICK_ALLOC_H

extern void quick_alloc_init(void);
extern void *quick_alloc(unsigned long);
extern void qa_free_all(void);

#endif
