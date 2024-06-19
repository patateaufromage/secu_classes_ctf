#define _GNU_SOURCE
#include "./include/my_alloc.private.h"
#include <linux/mman.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#define PAGE_SIZE 4096

static int log_file_descriptor = -1;

struct chunk *meta_heap = NULL;
struct chunk *data_heap = NULL;

// on rend laddresse du pool de data statique pour pouvoir lutiliser des linit de meta heap:
static void *data_heap_ptr = NULL;

// on met la meta_heap a 10000 pour eviter de la remap, on pourra donc faire 250+ chunks de meta
static size_t meta_heap_size = 12288;
size_t data_heap_size = 4096;

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////// LOGGING /// FUNCTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////


// fonction d'init
void initialize_log() {
  // if log init
  if (log_file_descriptor != -1) {
    return;
  }

  // getenv msm pour le nom du fichier de log
  const char *log_filename = getenv("MSM_OUTPUT");
  // si pas de log filename
  if (!log_filename) {
    return;
  }

  // ouverture en mode écriture, création et ajout
  log_file_descriptor = open(log_filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (log_file_descriptor == -1) 
  {
    perror("Failed to open log file");
    exit(EXIT_FAILURE);
  }
}


// fonction d'écriture fromat va list pour ecrire indefiniment
void write_log(const char *format, ...) {
  // init si pas deja fait
  if (log_file_descriptor == -1) {
    initialize_log();
  }

  va_list args;
  char *log_message_buffer;

  va_start(args, format);
  va_end(args);

  // alloca de 4096 pour etre tres large:
  log_message_buffer = alloca(4096);

  // on init la liste avec les bons parametres:
  va_start(args, format);
  vsnprintf(log_message_buffer, 4096, format, args);
  va_end(args);

  // write
  ssize_t bytes_written = write(log_file_descriptor, log_message_buffer, 4096);
  if (bytes_written == -1) {
    return;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// HEAP STUFF //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct chunk *init_meta_heap() {
  if (meta_heap == NULL) {
    // de base heap address sera 0x186a0000 = 409 600 000 en decimal
    meta_heap = (struct chunk *) mmap((void *)(PAGE_SIZE * 100000), meta_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    meta_heap->size = data_heap_size; // 4096 a la premiere initialisation
    meta_heap->flags = FREE; // sera set a BUSY au tout debut malloc, qui ici n'a pas encore ete realise
    meta_heap->prev = NULL;
    meta_heap->next = NULL;
    meta_heap->canary = 11114444;
    meta_heap->data_addr = data_heap_ptr;
    printf("[DEBUG] meta_heap canary : %d\n", meta_heap->canary);
    write_log("[DEBUG] meta_heap canary : %d\n", meta_heap->canary);
  }
  return meta_heap;
}


struct chunk *init_data_heap() {
  if (data_heap == NULL) {
    data_heap = mmap((void *)(PAGE_SIZE * 100000), data_heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    data_heap->canary = 11114444; // 0x215241111 ou -559038737
    printf("[DEBUG] data_heap canary : %d\n", data_heap->canary);
    write_log("[DEBUG] data_heap canary : %d\n", data_heap->canary);
  }
  return data_heap;
}


// la valeur retournee sera passee a "last_item" dans get_free_chunk()
// called using get_free_chunk()
struct chunk *get_last_chunk_raw() 
{
  while (meta_heap->next != 0 && meta_heap->flags == BUSY) 
  {
    printf("[_] Looking for last chunk ... item @ %p ... moving item until end\n", meta_heap);
    meta_heap = meta_heap->next;
  }
  return meta_heap;
}


// la valeur retournee sera passee a "item"
// called using get_free_chunk()
// this function will identify if a block at the end of the heap is free and of at least the same size than the one passed as an argument:
struct chunk *get_free_chunk_raw(size_t size) {
  // si la heap n'a pas ete initialisee dans get_free_chunk(), on le fait ici:
  if (meta_heap == NULL && data_heap == NULL) 
  {
    meta_heap = init_meta_heap();
    data_heap = init_data_heap();
    printf("[!] get_free_chunk() initialised meta_heap && data_heap !! sizeof(struct chunk) = %ld\n", sizeof(struct chunk));
    printf("[!] Beggining of meta space @ %p and data space @ %p\n", meta_heap, data_heap);
  }

  if (meta_heap->flags == FREE && meta_heap->size >= size) 
  {
    printf("[+] Enough space in data heap\n");
    
    // créaction du prochain chunk de la meta_heap:
    struct chunk *next_chunk = (struct chunk *)((size_t)meta_heap + sizeof(struct chunk));
    
    next_chunk->size = meta_heap->size - (size + sizeof(data_heap->canary));
    printf("[+] next_chunk->size = %zu\n", next_chunk->size);
    
    next_chunk->flags = FREE;
    printf("[+] next_chunk->flags = %d\n", next_chunk->flags);
    
    next_chunk->prev = meta_heap;
    printf("[+] next_chunk->prev = %p\n", next_chunk->prev);
    
    next_chunk->next = NULL;
    printf("[+] next_chunk->next = %p\n", next_chunk->next);
    
    next_chunk->canary = 11114444;
    printf("[+] next_chunk->canary = %d\n", next_chunk->canary);

    next_chunk->data_addr += size + sizeof(meta_heap->canary);
    printf("[+] next_chunk->data_addr = %p\n", next_chunk->data_addr);

    // mise à jour des variables:
    meta_heap->flags = BUSY;
    printf("[+] meta_heap->flags = BUSY\n");

    meta_heap->size = size;
    printf("[+] meta_heap->size = %zu\n", meta_heap->size);

    meta_heap->data_addr = data_heap;
    printf("[+] meta_heap->data_addr = %p\n", data_heap);

    meta_heap->next = next_chunk;
    printf("[+] meta_heap->next = %p\n", meta_heap->next);
    
    printf("\n");
    
    printf("[+] Free data block @ %p - bloc flag = %d - writeable bloc size = %lu\n", meta_heap->data_addr, meta_heap->flags, size);
    
    return meta_heap->data_addr;
  }
  printf("[+] No free block found @ %p\n", meta_heap);
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
    printf("[!] Begginin(struct chunk *) ((size_t)item + sizeof(struct chunk) + item->size)g of meta space @ %p and data space @ %p\n", meta_heap, data_heap);
  }

  struct chunk *item = get_free_chunk_raw(size); // malloc(8192) retournera NULL

  // cette condition sera remplie si l'espace data_heap est trop petit pour y
  // malloc
  if (item == NULL) 
  {
    printf("[!] Memory space not enough !\n");
    // we wanna know the place of the last chunk associated to last_item:
    struct chunk *last_item = get_last_chunk_raw();

    // preparation pour allocation de taille superieure a 4096:
    // malloc(8192) fera: total_size = 8192 + 16 = 8208
    size_t total_size = size + sizeof(struct chunk);
    // malloc(8192) fera: old_size = 4096
    size_t old_size = data_heap_size;

    // new_size sera un multiple de 4096:
    // ((total_size % 4096 != 0) ? 1 : 0) will check if the remainder is non
    // zero, and return 1 if True malloc(8192) fera: new_size = ((8208 / 4096)
    // + ((8208 % 4096 != 0) ? 1 : 0) * 4096 = (2 + 1) * 4096 = 3 * 4096 = 12288
    size_t new_size = ((total_size / 4096) + ((total_size % 4096 != 0) ? 1 : 0)) * 4096;

    // on ajoute la nouvelle partie de heap:
    data_heap_size = new_size; 
	// heap_size = 12288
	
    printf("[!] DATA HEAP new size will be : %lu\n", data_heap_size);

    // on remap l'ancienne heap (de size old_size) pour remplacer sa size par la nouvelle size calculee:
    struct chunk *new_data_heap = mremap(data_heap, old_size, data_heap_size, MREMAP_MAYMOVE);
    printf("[!] Remap DONE, new_heap points to %p\n", new_data_heap);

    // check si la nouvelle heap pointe au meme endroit que lancienne, sinon on return NULL, qui sera utilise via get_free_chunk_raw(size) pour reinit une heap:
    if (new_data_heap != data_heap) {
      return NULL;
    }

    last_item->size = new_size;

    printf("[!] Resizing DONE, last_item points @ %p\n", last_item);
    printf("[!] Data size available = %lu - flag = %u\n", last_item->size, last_item->flags);

    // on reassocie item via get free chunk raw ce qui permet de lui redonner la nouvelle addresse calculee, a savoir, item sera place au debut de l'element libre nouvellement cree:
    printf("[!] Reassigning item new properties ...\n");
    item = get_free_chunk_raw(size);
    printf("[!] Done, new allocated space begins @ %p\n", item);
  }
  // item pointera au debut de l"espace libre:
  return item;
}


void *my_alloc(size_t size) 
{
  // on cast size en void au cas on ne sen sert pas dans la fonction, afin de ne pas creer derreur de compilo
  (void)size;

  printf("\n[*] Debut de my_alloc(%lu)\n", size);

  struct chunk *ch = get_free_chunk(size);
  printf("[*] Chunk data initialise @ %p\n", ch);

  printf("[*] my_alloc TERMINE !\n");
  printf("[*] ptr zone data @ %p\n", ch);
  printf("[*] ... END of malloc ...\n");
  return ch;
}


void clean(void *ptr)
{
  struct chunk *ch = (struct chunk*) ((size_t)ptr - sizeof(struct chunk));
  ch->flags = FREE;

  // on deplace item vers la fin de l'espace alloue:
  for (struct chunk *item = heap; (size_t)item < (size_t)heap + heap_size;item = (struct chunk *)((size_t)item //+ item->size + sizeof(struct
  chunk))
  {
    printf("[.] Checking for free block ...\n");
    if (item->flags == FREE)
    {
      struct chunk *end = item;
      printf("[.] Free block detected @ %p\n", end);

      size_t new_size = item->size;

      // as long as we dont reach the very-end, moving around with "end":
      while (end->flags == FREE && (size_t)end + sizeof(struct chunk) +
      end->size < (size_t)heap + heap_size)
      {
        end = (struct chunk*) ((size_t)end + end->size + sizeof(struct
        chunk)); if (end->flags == FREE)
        {
          new_size += end->size + sizeof(struct chunk);
          printf("[.] Free space found ...\n");
        }
        printf("[.] CLEANING DONE, new size %lu - end @ %p - following
        block is of size %lu - flag %u\n", new_size, end, end->size,
        end->flags);
      }

      // merge les chunks consecutifs
      item->size = new_size;
    }
  }
}
