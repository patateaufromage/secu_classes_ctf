#include <criterion/criterion.h>
#include <sys/mman.h>
#include "my_alloc.private.h"
#include <stdio.h>

/* #include "my_alloc.private.h" */

Test(simple, simple_map_01)
{
	printf("\nsimple_mapping_JUSTforFUN_01\n");
	char *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	cr_assert(ptr != NULL, "Failed to mmap");
}

Test(simple, simple_map_02)
{
	printf("\nsimple_malloc_02\n");
	
	char *ptr1 = my_alloc(12);
	cr_assert(ptr1 != NULL, "Failed to alloc ptr1");
	char *ptr2 = my_alloc(25);
	cr_assert(ptr2 != NULL, "Failed to alloc ptr2");
	
	cr_assert((size_t)ptr2 == (size_t)ptr1 + 12 + (sizeof (struct chunk)), "Failed to alloc"); 
	char *ptr3 = my_alloc(55);
	cr_assert((size_t)ptr3 == (size_t)ptr2 + 25 + (sizeof (struct chunk)), "Failed to alloc");
}

Test(simple, simple_map_03)
{
	printf("\nsimple_malloc_03\n");
	
	char *ptr1 = my_alloc(12);
	cr_assert(ptr1 != NULL, "Failed to alloc ptr1");
	char *ptr2 = my_alloc(25);
	cr_assert(ptr2 != NULL, "Failed to alloc ptr2");
	
	cr_assert((size_t)ptr2 == (size_t)ptr1 + 12 + (sizeof (struct chunk)), "Failed to alloc"); 
	
	char *ptr3 = my_alloc(55);
	cr_assert((size_t)ptr3 == (size_t)ptr2 + 25 + (sizeof (struct chunk)), "Failed to alloc");	
	
	clean(ptr1);
	printf("ptr1 cleaned\n");
        clean(ptr2);
	printf("ptr2 cleaned\n");
	
	struct chunk *t = (struct chunk *)((size_t)ptr1 - sizeof (struct chunk));
	printf("t : %lu\n", t->size);
	cr_assert(t->size == 12 + 25 + sizeof(struct chunk), "Failed to clean");
}

Test(simple, simple_map_04)
{
	printf("\nsimple_malloc_04\n");
	
	char *ptr1 = my_alloc(12);
	cr_assert(ptr1 != NULL, "Failed to alloc ptr1");
	char *ptr2 = my_alloc(25);
	cr_assert(ptr2 != NULL, "Failed to alloc ptr2");
	cr_assert((size_t)ptr2 == (size_t)ptr1 + 12 + (sizeof (struct chunk)), "Failed to alloc");
	
	char *ptr3 = my_alloc(55);
	cr_assert(ptr3 != NULL, "Failed to alloc ptr3");
	cr_assert((size_t)ptr3 == (size_t)ptr2 + 25 + (sizeof (struct chunk)), "Failed to alloc");
	
	clean(ptr2);
	printf("ptr2 cleaned\n");
	clean(ptr3);
	printf("ptr3 cleaned\n");
	
	struct chunk *t = (struct chunk*)((size_t)ptr2 - sizeof(struct chunk));
	printf("t->size = %ld\n", t->size);
	cr_assert(t->size == 25 + 55 + 3940 + 2 * sizeof(struct chunk), "Failed to clean");
}

Test(simple, simple_map_05)
{
	printf("\nsimple_malloc_05\n");
	
	char *ptr1 = my_alloc(12);
	cr_assert(ptr1 != NULL, "Failed to alloc ptr1");
	char *ptr2 = my_alloc(25);
	cr_assert(ptr2 != NULL, "Failed to alloc ptr2");
	cr_assert((size_t)ptr2 == (size_t)ptr1 + 12 + (sizeof (struct chunk)), "Failed to alloc");

	char *ptr3 = my_alloc(55);
	cr_assert((size_t)ptr3 == (size_t)ptr2 + 25 + (sizeof (struct chunk)), "Failed to alloc");

	clean(ptr1);
	printf("ptr1 cleaned\n");
	clean(ptr2);
	printf("ptr2 cleaned\n");
	
	struct chunk *t = (struct chunk*)((size_t)ptr1 - sizeof(struct chunk));
	printf("t->size = %ld\n", t->size);
	cr_assert(t->size == 12 + 25 + sizeof(struct chunk), "Failed to clean ptr2");
}

Test(simple, simple_map_06)
{
	printf("\nsimple_map_06\n");
	char *ptr1 = my_alloc(8192);
	cr_assert(ptr1 != NULL, "Failed to alloc ptr1");
}
