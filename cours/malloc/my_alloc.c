#define _GNU_SOURCE
#include "my_alloc.private.h"
#include <stdio.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define PAGE_SIZE 4096


// Will include a size of type size_t and a flag of type enum chunk_type:
// From man mmap: 
	// If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping; this is the most portable method of 
	// creating a new mapping.  If addr is not NULL, then the kernel takes it as a hint about where to place the mapping;
struct chunk *heap = NULL;


// We assign to this variable a size, that we will use with *heap
// When casting using size_t we convert in int aka in decimal
size_t heap_size = 4096;


// mapping d'une heap de taille predefinie, et non passee en argument de malloc a ce point, de 409 600 000
struct chunk *init_heap()
{
	if (heap == NULL)
	{
		// le mappage de zone sera fait automatiquement etant donne quon a passe NULL en valeur a heap: 
		// from man mmap: 
			// For mmap(), offset must be a multiple of the underlying huge page size.  The system automatically aligns length to be a multiple of the underlying huge page size.
		// de base heap address sera 0x186a0000 = 409 600 000 en decimal 
		heap = mmap( (void*) (PAGE_SIZE * 100000), heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0 );

		// on "inscrit" la valeur de la taille dispo, ainsi que le flag indiquant si lespace sera libre ou non, qui seront integres au chunk "ch" de metadata.
		// la taille a allouer, qui sera indiquee dans le chunk de meta, sera egale a la taille totale creee, moins la taille du chunk contenant les metas:
		heap->size = heap_size - sizeof(struct chunk);
		heap->flags = FREE;  // FREE = 0
	}

	return heap;
}


// la valeur retournee sera passee a "last_item" dans get_free_chunk()
// called using get_free_chunk()
// determinera la position du dernier chunk et recup son flag 
struct chunk *get_last_chunk_raw()
{
	// item est un pointer et va nous servir a nous ballader dans la heap, et donc on le declare comme egal a la heap en tous points.
	// la condition for: pour item du meme type que heap, tant qu'il ne pointe pas plus loin que heap + 4096, alors on le decale de chunk + size allouee:
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size))
	{

		printf("[_] Looking for last chunk ... item @ %p ... moving item until end\n", item);
		printf("[_] End reached @ %p !\n", (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)));
		
		// si on reach la fin de la heap, on return item: 
		if ((size_t)item + sizeof(struct chunk) + item->size >= (size_t)heap + heap_size)
		{	
			printf("[_] Space not sufficient for allocation, resizing !\n");		
			return item;
		}

		printf("[end not reached, skipping ...]\n");
	}
	// si la condition "for" n'est pas validee, alors item nest pas return et on return NULL:
	return NULL;
}


// la valeur retournee sera passee a "item"
// called using get_free_chunk()
// this function will identify if a block at the end of the heap is free and of bigger size than the one passed as an argument:
struct chunk *get_free_chunk_raw(size_t size)
{
	// si la heap n'a pas ete initialisee dans get_free_chunk(), on le fait ici:
	if (heap == NULL)
	{
		heap = init_heap();
		printf("[+] get_free_chunk_raw() initialised heap @ %p\n", heap); 
		printf("[+] Data size available = %lu - flag = %u\n", heap->size, heap->flags);
	}
	
	// on deplace item vers la fin de l'espace de base alloue:
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size))
	{
		printf("[+] item reached end of allocated space @ %p\n", (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)));

		if (item->flags == FREE && item->size >= size)
		{
			printf("[+] Free space found ... assigning it to item !\n");
			return item;
		}
	}
	
	// si on a pas assez d'espace, comme au premier malloc(8192), on return NULL:
	return NULL;
}

// la valeur retournee sera passee a "ch"
// will call get_free_chunk_raw() and return item or NULL
struct chunk *get_free_chunk(size_t size)
{
	// if we are at the first use of the program, heap == NULL
	if (heap == NULL)
	{
		heap = init_heap();
		printf("[!] get_free_chunk() initialised heap @ %p\n", heap);
		printf("[!] Data size available = %lu - flag = %u\n", heap->size, heap->flags); 
	}

	// creation de item, associe au debut de l'espace memoire libre restant.
	struct chunk *item = get_free_chunk_raw(size); // malloc(8192) retournera NULL

	if (item == NULL)
	{
		printf("[!] Memory space not enough !\n");		
		// we wanna know the place of the last chunk associated to last_item:
		struct chunk *last_item = get_last_chunk_raw();
		
		// preparation pour allocation de taille superieure a 4096:
		// malloc(8192) fera: total_size = 8192 + 16 = 8208
		size_t total_size = size + sizeof(struct chunk);
		// malloc(8192) fera: old_size = 4096
		size_t old_size = heap_size;
		
		// delta_size sera un multiple de 4096:
		// ((total_size % 4096 != 0) ? 1 : 0) will check if the remainder is non zero, and return 1 if True
		// malloc(8192) fera: delta_size = ((8208 / 4096) + ((8208 % 4096 != 0) ? 1 : 0) * 4096 = (2 + 1) * 4096 = 3 * 4096 = 12288 
		size_t delta_size = ((total_size / 4096) + ((total_size % 4096 != 0) ? 1 : 0)) * 4096;

		// on ajoute la nouvelle partie de heap:
		// 4096 += 12288 = 16384
		// lionel used this formula, which is useless actually : heap_size += delta_size;
		heap_size = delta_size; // this one is enough, the paging needs to be a multiple of 4096, but we wanna create as less unused space as possible !
		// heap_size = 12288
		printf("[!] HEAP new size will be : %lu\n", heap_size);

		// on remap l'ancienne heap (de size old_size) pour remplacer sa size par la nouvelle size calculee:
		struct chunk *new_heap = mremap(heap, old_size, heap_size, MREMAP_MAYMOVE);
		printf("[!] Remap DONE, new_heap points to %p\n", new_heap);
		
		// check si la nouvelle heap pointe au meme endroit que lancienne, sinon on return NULL, qui sera utilise via get_free_chunk_raw(size) pour reinit une heap: 
		if (new_heap != heap)
			return NULL;

		// on rajoute la delta_size precedemment calculee pour deplacer last_item au dernier bloc: 
		// changed also here from lionel stuff: last_item->size += delta_size;
		last_item->size = delta_size; 

		printf("[!] Resizing DONE, last_item points @ %p\n", last_item);
		printf("[!] Data size available = %lu - flag = %u\n", last_item->size, last_item->flags);
		
		// on reassocie item via get free chunk raw ce qui permet de lui redonner la nouvelle addresse calculee, a savoir, item sera place au debut de l'element libre nouvellement cree:
		printf("[!] Reassigning item new properties ...\n");
		item = get_free_chunk_raw(size);
		printf("[!] New space beginning @ %p\n", item);
	}
	return item;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// all calculations and else are performed in comments using my_alloc(8192) ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void *my_alloc(size_t size) {
	// on cast size en void au cas on ne sen sert pas dans la fonction, afin de ne pas creer derreur de compilo 
	(void) size;
	void *ptr;
	
	printf("[*] Beginning of my_alloc(%lu)\n", size);

	// creation de ch qui contiendra les metadatas, et pointera vers le debut de l'emplacement libre de taille passee en argument
	struct chunk *ch = get_free_chunk(size);
	
	// ptr pointera vers le debut du chunk libre de taille passee en argument
	// on cast l'ensemble en void* pour pouvoir output dans le meme type que ptr
	ptr = (void*) ((size_t)ch + sizeof(struct chunk));
	printf("[*] Beginning of data part @ %p\n", ptr);
	
	// on associe la fin de la partie allouee a "end":
	struct chunk *end = (struct chunk*) ((size_t)ptr + size);
	end->flags = FREE;
	end->size = ch->size - sizeof(struct chunk) - size;
	printf("[*] End of data part @ %p\n", end);
	printf("[*] Beginning of last data space @ %p\n", (struct chunk *) ((size_t)end + (sizeof(struct chunk))));
	printf("[*] Data space available = %lu - flag = %u \n", end->size, end->flags); 
	
	ch->flags = BUSY;
	ch->size = size;	
	printf("[*] my_alloc DONE !\n");
	return ptr;
}


void clean(void *ptr)
{
	struct chunk *ch = (struct chunk*) ((size_t)ptr - sizeof(struct chunk));
	ch->flags = FREE;
	
	// on deplace item vers la fin de l'espace alloue:
	for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size; item = (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)))
	{
		printf("[.] Checking for free block ...\n");
		if (item->flags == FREE)
		{
			struct chunk *end = item;
			printf("[.] Free block detected @ %p\n", end);
			
			size_t new_size = item->size;
			
			// as long as we dont reach the very-end, moving around with "end":
			while (end->flags == FREE && (size_t)end + sizeof(struct chunk) + end->size < (size_t)heap + heap_size)
			{
				end = (struct chunk*) ((size_t)end + end->size + sizeof(struct chunk));
				if (end->flags == FREE)
				{
					new_size += end->size + sizeof(struct chunk);
					printf("[.] Free space found ...\n");
				}
				printf("[.] CLEANING DONE, new size %lu - end @ %p - following block is of size %lu - flag %u\n", new_size, end, end->size, end->flags);
			}
			
			// merge les chunks consecutifs
			item->size = new_size;
		}
	}
}
