#define _GNU_SOURCE
#include "my_alloc.private.h"
#include <stdio.h>
#include <sys/mman.h>
#include <linux/mman.h>

#define PAGE_SIZE 4096


struct chunk *meta_heap = NULL;
struct chunk *data_heap = NULL;


size_t meta_heap_size = 4096;
size_t data_heap_size = 4096;


struct chunk *init_meta_heap()
{
	if (meta_heap == NULL)
	{
		// de base heap address sera 0x186a0000 = 409 600 000 en decimal
		meta_heap = mmap( (void*) (PAGE_SIZE * 100000), meta_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0 );
		meta_heap->size = meta_heap_size;
		meta_heap->flags = FREE;  
		meta_heap->prev = NULL;
		meta_heap->data_addr = NULL;
		meta_heap->next = NULL;
	}
	return meta_heap;
}


struct chunk *init_data_heap()
{
	if (data_heap == NULL)
	{
		// du coup where is data_heap ??
		data_heap = mmap( (void*) (PAGE_SIZE * 100000), data_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0 );
		data_heap->size = data_heap_size;
		data_heap->flags = FREE;
		data_heap->prev = NULL;
		data_heap->data_addr = NULL;
		data_heap->next = NULL;
	}
	return data_heap;
}


// la valeur retournee sera passee a "last_item" dans get_free_chunk()
// called using get_free_chunk()
struct chunk *get_last_chunk_raw()
{
	for (struct chunk *item = meta_heap; (size_t)item < (size_t)meta_heap + meta_heap_size; item = (struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size))
	{

		printf("[_] Looking for last chunk ... item @ %p ... moving item until end\n", item);
		printf("[_] End reached @ %p !\n", (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)));
		
		// si on reach la fin de la heap, on return item: 
		if ((size_t)item + sizeof(struct chunk) + item->size >= (size_t)meta_heap + meta_heap_size)
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
// this function will identify if a block at the end of the heap is free and of at least the same size than the one passed as an argument:
struct chunk *get_free_chunk_raw(size_t size)
{
	// si la heap n'a pas ete initialisee dans get_free_chunk(), on le fait ici:
	if (meta_heap == NULL && data_heap == NULL)
	{
		meta_heap = init_meta_heap();
		data_heap = init_data_heap();
		printf("[!] get_free_chunk() initialised meta_heap && data_heap !!\n");
		printf("[!] Beggining of meta space @ %p and data space @ %p\n", meta_heap, data_heap);
	}
	
	// on deplace item vers la fin de l'espace de base alloue:
	for (struct chunk *item = meta_heap; (size_t)item < (size_t)meta_heap + meta_heap_size; item = (struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size))
	{
		printf("[+] End of allocated space found @ %p\n", (struct chunk *) ((size_t)item + item->size + sizeof(struct chunk)));
		printf("[+] ... Checking for free blocks ...\n");

		if (item->flags == FREE && item->size >= size)
		{
			printf("[+] Free block @ %p - bloc flag = %u - writeable bloc size = %lu - size passed = %lu \n", item, item->flags, item->size, size);
			return item;
		}

		printf("[+] No free block found @ %p\n", item);
	}
	
	// si on a pas assez d'espace, comme au premier malloc(8192), on return NULL:
	return NULL;
}

// la valeur retournee sera passee a "ch"
// will call get_free_chunk_raw() and return item or NULL
struct chunk *get_free_chunk(size_t size)
{
	// if we are at the first use of the program, heap == NULL
	if (meta_heap == NULL && data_heap == NULL)
	{
		meta_heap = init_meta_heap();
		data_heap = init_data_heap();
		printf("[!] get_free_chunk() initialised meta_heap && data_heap !!\n");
		printf("[!] Beggining of meta space @ %p and data space @ %p\n", meta_heap, data_heap);
	}

	// on rempli le premier bloc de meta avec laddresse maintenant connue de data_ptr:
	//meta_heap->

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
		size_t old_size = meta_heap_size;
		
		// delta_size sera un multiple de 4096:
		// ((total_size % 4096 != 0) ? 1 : 0) will check if the remainder is non zero, and return 1 if True
		// malloc(8192) fera: delta_size = ((8208 / 4096) + ((8208 % 4096 != 0) ? 1 : 0) * 4096 = (2 + 1) * 4096 = 3 * 4096 = 12288 
		size_t delta_size = ((total_size / 4096) + ((total_size % 4096 != 0) ? 1 : 0)) * 4096;

		// on ajoute la nouvelle partie de heap:
		// 4096 += 12288 = 16384
		// lionel used this formula, which is useless actually : heap_size += delta_size;
		meta_heap_size = delta_size; // this one is enough, the paging needs to be a multiple of 4096, but we wanna create as less unused space as possible !
		// heap_size = 12288
		printf("[!] HEAP new size will be : %lu\n", meta_heap_size);

		// on remap l'ancienne heap (de size old_size) pour remplacer sa size par la nouvelle size calculee:
		struct chunk *new_meta_heap = mremap(meta_heap, old_size, meta_heap_size, MREMAP_MAYMOVE);
		printf("[!] Remap DONE, new_heap points to %p\n", new_meta_heap);
		
		// check si la nouvelle heap pointe au meme endroit que lancienne, sinon on return NULL, qui sera utilise via get_free_chunk_raw(size) pour reinit une heap: 
		if (new_meta_heap != meta_heap)
			return NULL;

		// on rajoute la delta_size precedemment calculee pour deplacer last_item au dernier bloc: 
		// changed also here from lionel stuff: last_item->size += delta_size;
		last_item->size = delta_size; 

		printf("[!] Resizing DONE, last_item points @ %p\n", last_item);
		printf("[!] Data size available = %lu - flag = %u\n", last_item->size, last_item->flags);
		
		// on reassocie item via get free chunk raw ce qui permet de lui redonner la nouvelle addresse calculee, a savoir, item sera place au debut de l'element libre nouvellement cree:
		printf("[!] Reassigning item new properties ...\n");
		item = get_free_chunk_raw(size);
		printf("[!] Done, new allocated space begins @ %p\n", item);
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
	// ch sera associe a item:
	struct chunk *ch = get_free_chunk(size);
	printf("[*] Chunk initialised @ %p\n", ch);
	
	// ptr pointera a la fin de ch, aka la fin de la partie metadate, et donc au debut de la partie data:
	ptr = (void*) ((size_t)ch + sizeof(struct chunk));
	printf("[*] Fin du chunk et debut de l'espace data @ %p\n", ptr);
	
	// "end" pointe vers la fin de chunk + data:
	struct chunk *end = (struct chunk*) ((size_t)ptr + size);
	end->flags = FREE;
	end->size = ch->size - sizeof(struct chunk) - size;
	
	printf("[*] Data block ends @ %p\n", (struct chunk *) ((size_t)end));
	printf("[*] Data space available = %lu - flag = %u \n", end->size, end->flags); 
	
	ch->flags = BUSY;
	ch->size = size;	

	printf("[*] my_alloc DONE !\n");
	printf("[*] ptr @ %p\n", ptr);
	printf("[*] ...\n");
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
