#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifndef POOL_H
#    define POOL_H
#    ifndef DEFAULT_ALIGNMENT
#        define DEFAULT_ALIGNMENT (2 * sizeof(void *))

#    endif  // DEFAULT_ALIGNMENT

//declarations
typedef struct Pool_Free_Node Pool_Free_Node;

struct Pool_Free_Node {
    Pool_Free_Node* next;
};

typedef struct {
    unsigned char* buffer;
    size_t buffer_size;
    size_t chunk_size;
    Pool_Free_Node* head;

} Pool;

static inline void pool_init(Pool* pool, void* buffer, size_t buffer_size,
                             size_t chunk_size);

static inline void pool_init_aligned(Pool* pool, void* buffer,
                                     size_t buffer_size, size_t chunk_size,
                                     size_t chunk_alignment);
static inline Pool_Free_Node* pool_alloc(Pool* pool, size_t num);

uintptr_t align_forward_ptr(uintptr_t base_ptr, uintptr_t chunk_alignment);
size_t align_forward_chunk_size(size_t chunk_size, size_t chunk_alignment);

#    define POOL_IMPLEMENTATION

#    ifdef POOL_IMPLEMENTATION

static inline void
pool_init(Pool* pool, void* buffer, size_t buffer_size, size_t chunk_size)
{
    pool_init_aligned(pool, buffer, buffer_size, chunk_size, DEFAULT_ALIGNMENT);
}

static inline void
pool_init_aligned(Pool* pool, void* buffer, size_t buffer_size,
                  size_t chunk_size, size_t chunk_alignment)
{
    uintptr_t base_ptr = (uintptr_t)buffer;
    uintptr_t start = align_forward_ptr(base_ptr, (uintptr_t)chunk_alignment);

    buffer_size -= (size_t)(start - base_ptr);
    chunk_size = align_forward_chunk_size(chunk_size, chunk_alignment);

    assert(chunk_size >= sizeof(Pool_Free_Node) && "Chunk size is too small");
    assert(buffer_size >= chunk_size
           && "buffer size is smaller than chunk size");

    pool->buffer      = (unsigned char*)buffer;
    pool->buffer_size = buffer_size;
    pool->chunk_size  = chunk_size;
    pool->head        = NULL;
}

uintptr_t
align_forward_ptr(uintptr_t base_ptr, uintptr_t chunk_alignment)
{
}

size_t
align_forward_chunk_size(size_t chunk_size, size_t chunk_alignment)
{
}
#    endif  // POOL_IMPLEMENTATION
#endif      //POOL_H
