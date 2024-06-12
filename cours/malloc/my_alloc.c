#define _GNU_SOURCE
#include "my_alloc.private.h"
#include <stdio.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define PAGE_SIZE 4096


// will include a size of type size_t and a flag of type enum chunk_type:
// from man mmap: 
	// If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping; this is the most portable method of 
	// creating a new mapping.  If addr is not NULL, then the kernel takes it as a hint about where to place the mapping;
struct chunk *heap = NULL;
// we assign to this variable a size, not yet linked to *heap:
size_t heap_size = 4096;


// mapping d'une heap de taille 409 600 000
struct chunk *init_heap()
{
	if (heap == NULL)
	{
		// on sait que la zone addresse ou notre malloc allouera de la memoire contient largement assez de place, donc on prevoit large:
		heap = mmap( (void*) (PAGE_SIZE * 100000), heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0 );
		// on determine la taille reelle disponible, ainsi que le flag indiquant si lespace sera libre ou non, qui sera integree au chunk de metadata.
		// la taille a allouer, qui sera indiquee dans le chunk de meta, sera egale a la taille totale creee, moins la taille du chunk contenant les metas:
		heap->size = heap_size - sizeof(struct chunk);
		heap->flags = FREE;  // = 0
	}
	return heap;
}


struct chunk *get_last_chunk_raw()
{
	// item va nous serir a nous ballader dans la heap, cest un pointer, donc on le declare comme etant constitue comme la heap et donc du meme type.
	// a son initialisation il pointe vers NULL	
	// tant que item est du meme type que heap, que sa taille ne depasse pas la taille de la heap totale, alors on le decale de chunk + size allouee:
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size))
	{
		printf("[_] Checking for last chunk, item @ %p - size %lu - flag %u\n", item, item->size, item->flags);
		printf("[_] (size_t)item = %lu - size_t(heap) = %lu", (size_t)item, (size_t)heap);
		// si on reach la fin de la heap, on return aka on ret 
		if ((size_t)item + sizeof(struct chunk) + item->size >= (size_t)heap + heap_size)
		{	
			printf("[_] Heap end reached, returning from position %p\n", item);
			return item;
		}
		printf("[skip]\n");
	}
	// si on est en dehors de la zone de heap, on return NULL:
	return NULL;
}


// this function will identify if a block at the end of the heap is free but of smaller size than the one we passed as an argument:
struct chunk *get_free_chunk_raw(size_t size)
{
	if (heap == NULL)
	{
		heap = init_heap();
	}
	// on avance dans la heap en deplacant le pointer item:
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)))
	{
		// si on deborde de la heap, on return item:
		if (item->flags == FREE && item->size >= size)
			return item;
	}
	// si on est en dehors de la zone de heap, on return NULL:
	return NULL;
}


// get_free_chunk va init la heap si besoin, resize si besoin, et se servir de get_free_chunk_raw pour  pointer vers le debut de lespace libre:
struct chunk *get_free_chunk(size_t size)
{
	if (heap == NULL)
	{
		heap = init_heap();
	}
	printf("[>] HEAP initialised, starting point @ %p\n", heap);

	// recup de la position de item, associe a l'espace memoire libre restant:
	struct chunk *item = get_free_chunk_raw(size);
	
	// get_free_chunk_raw() return NULL si on depasse la zone memoire, donc ici on remap l'espace memoire en ajoutant le nombre de page necessaires, 
	// qui sont des multiples de 4096:
	// from man mmap: 
		// For mmap(), offset must be a multiple of the underlying huge page size.  The system automatically aligns length to be a multiple of the underlying huge page size.
	if (item == NULL)
	{
		printf("[!] Memory space not enough,  item value returned was %p ... RESIZING !\n", item);

		// preparation pour allocation de taille superieure a 4096:
		size_t total_size = size + sizeof(struct chunk);
		size_t old_size = heap_size;
		// delta_size sera un multiple de 4096:
		// ((total_size % 4096 != 0) ? 1 : 0) will check if the remainder is non zero, and return 1 if True
		size_t delta_size = ((total_size/4096) + ((total_size % 4096 != 0) ? 1 : 0)) * 4096;
		// we wanna know the place of the last chunk associated to last_item:
		struct chunk *last_item = get_last_chunk_raw();

		// on ajoute la nouvelle partie de heap:
		heap_size += delta_size;
		printf("[!] HEAP new size will be : %lu\n", heap_size);

		// on remap l'ancienne heap (de size old_size) pour y inclure la nouvelle size associee a heap_size, et on l'associe a new_heap:
		struct chunk *new_heap = mremap(heap, old_size, heap_size, MREMAP_MAYMOVE);
		printf("[!] Remap done, resized, value is : %p\n", new_heap);
		
		// on check si la nouvelle heap pointe au meme endroit que lancienne, si cest pas le cas on fait pointer vers NULL pour choper un espace memoire auto quand mmap sera appele:
		if (new_heap != heap)
			return NULL;

		// on rajoute la delta_size precedemment calculee, ce qui permet de definir la taille du last item comme etant  agrandie:
		last_item->size += delta_size; 
		printf("[!] RESIZING DONE, last_item now @ %p - size : %lu - flag : %u\n", last_item, last_item->size, last_item->flags);
		
		// on reassocie item via get free chunk raw ce qui permet de lui redonner la nouvelle addresse calculee, a savoir, item sera place au debut de l'element libre nouvellement cree:
		item = get_free_chunk_raw(size);
		printf("[!] Free chunk item is now @ %p\n", item);
	}
	return item;
}


void *my_alloc(size_t size) {
	// on cast size en void au cas on ne sen sert pas dans la fonction, afin de ne pas creer derreur de compilo 
	(void) size;
	printf("[*] my_alloc will be tried using size %lu\n", size);
	void *ptr;
	
	// ch qui contiendra les metadatas, pointera vers le debut de l'emplacement libre de taille passee en argument
	struct chunk *ch = get_free_chunk(size);
	
	// ptr pointera vers la fin du chunk libre de taille passee en argument
	// on cast l'ensemble en void* pour pouvoir output dans le meme type que ptr
	ptr = (void*) ((size_t)ch + sizeof(struct chunk));
	printf("[*] ptr pointe vers fin du chunk libre, de taille %lu passee en argument, localise @ %p\n", size, ptr);
	
	// on associe la toute fin de la heap au pointer end:
	struct chunk *end = (struct chunk*) ((size_t)ptr + size);

	// 
	end->flags = FREE;
	end->size = ch->size - sizeof(struct chunk) - size;
	ch->flags = BUSY;
	ch->size = size;
	
	printf("[*] my_alloc done !\n");
	return ptr;
}


void clean(void *ptr)
{
	struct chunk *ch = (struct chunk*) ((size_t)ptr - sizeof(struct chunk));
	// lookp ptr : idee de truc secu a faire
	ch->flags = FREE;
	// merge les chunks consecutifs
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)))
	{
		printf("[.] CLEANING ? checking chunk @ %p - size %lu - flag %u\n", item, item->size, item->flags);
		if (item->flags == FREE)
		{
			// voir les blocs consecutifs
			struct chunk *end = item;
			size_t new_size = item->size;
			while (end->flags == FREE && (size_t)end + sizeof(struct chunk) + end->size < (size_t)heap + heap_size)
			{
				end = (struct chunk*) ((size_t)end + end->size + sizeof(struct chunk));
				if (end->flags == FREE)
				{
					new_size += end->size + sizeof(struct chunk);
				}
				printf("[.] CLEANING DONE, new size %lu - end @ %p - consecutive blocks %lu - flag %u\n", new_size, end, end->size, end->flags);
			}
			item->size = new_size;
		}
	}
}

