#ifndef __MY_ALLOC_PRIVATE_H
#define __MY_ALLOC_PRIVATE_H
#include "my_alloc.h"
#include <stddef.h>

// this program provides initial setup to manage memory chunks:

enum chunk_type
{
	FREE = 0,
	BUSY = 1
};


struct chunk
{
	size_t size;
	enum chunk_type flags;
	int canary;
	struct chunk *prev;
	struct chunk *next;
	struct chunk *data_addr;
};


struct chunk *init_meta_heap();
struct chunk *init_data_heap();

struct chunk *get_free_chunk(size_t s);
struct chunk *get_free_chunk_raw(size_t size);
struct chunk *get_last_chunk_raw();

void *my_alloc(size_t size);
void clean(void *ptr);

#endif
