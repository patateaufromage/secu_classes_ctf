#ifndef __MY_ALLOC_PRIVATE_H
#define __MY_ALLOC_PRIVATE_H
#include "my_alloc.h"
#include <stddef.h>

enum chunk_type
{
	FREE = 0,
	BUSY = 1
};


struct chunk
{
	size_t size;
	enum chunk_type flags;
};

struct chunk *init_heap();
struct chunk *get_last_chunk_raw();
struct chunk *get_free_chunk_raw(size_t size);
struct chunk *get_free_chunk(size_t s);
void *my_alloc(size_t size);
void clean(void *ptr);

#endif
