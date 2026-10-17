#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#ifndef ARENA_H
#    define ARENA_H
#    ifndef DEFAULT_ALIGNMENT
#        define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#    endif  // !DEFAULT_ALIGNMENT


// ----Arena----//
typedef struct {
    unsigned char* buffer;
    size_t buffer_size;
    size_t offset;
    size_t prev_offset;
} Arena;

void* arena_alloc(Arena* arena, size_t size, size_t align = DEFAULT_ALIGNMENT);

void* arena_resize_aligned(Arena* arena, void* buffer, size_t curr_size,
                           size_t new_size, size_t align);

void* arena_resize(Arena* arena, void* buffer, size_t curr_size,
                   size_t new_size, size_t align = DEFAULT_ALIGNMENT);

void arena_init(Arena* arena, void* buffer, size_t buffer_size);

void arena_reset(Arena* arena);
void arena_free(Arena* arena);

/*
 * utility functions
 * */
bool arena_is_power_of_two(uintptr_t);
uintptr_t arena_align_forward(uintptr_t ptr, size_t align);

// ----TempArena ----//

typedef struct {
    Arena* arena;
    size_t offset;
    size_t prev_offset;
} ArenaTemp;

ArenaTemp arena_savepoint_create(Arena*);
void arena_savepoint_restore(ArenaTemp);

////////////////////////
#    ifdef ARENA_IMPLEMENTATION

bool
arena_is_power_of_two(uintptr_t align)
{
    return (align & (align - 1)) == 0;
}

uintptr_t
arena_align_forward(uintptr_t ptr, size_t align)
{
    uintptr_t p, a, modulo;

    assert(arena_is_power_of_two(align));

    p      = ptr;
    a      = (uintptr_t)align;
    modulo = p & (a - 1);

    if (modulo != 0) {
        p += a - modulo;
    }
    return p;
}
// -------------------------------------------

static inline void*
arena_alloc(Arena* arena, size_t size)
{
    arena_alloc_aligned(arena, size, DEFADEFAULT_ALIGNMENT);
}

static inline void*
arena_alloc_aligned(Arena* arena, size_t size, size_t align)
{
    uintptr_t curr_ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;
    uintptr_t offset   = arena_align_forward(curr_ptr, align);

    offset -= (uintptr_t)arena->buffer;

    if (offset + size <= arena->buffer_size) {
        void* ptr          = &arena->buffer[offset];
        arena->prev_offset = offset;
        arena->offset      = offset + size;

        memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}

void*
arena_resize(Arena* arena, void* buffer, size_t curr_size, size_t new_size,
             size_t align)
{
    unsigned char* old_mem = (unsigned char*)buffer;

    assert(arena_is_power_of_two(align));

    if (old_mem == NULL || curr_size == 0) {
        return arena_alloc(arena, new_size, align);

    } else if (arena->buffer <= old_mem
               && old_mem < arena->buffer + arena->buffer_size) {
        if (arena->buffer + arena->prev_offset == old_mem) {
            if (arena->prev_offset + new_size <= arena->buffer_size) {
                arena->offset = arena->prev_offset + new_size;
                if (new_size > curr_size) {
                    memset(&arena->buffer[arena->prev_offset + curr_size], 0,
                           new_size - curr_size);
                }
                return buffer;
            }
            return NULL;
        } else {
            void* new_buffer = arena_alloc(arena, new_size, align);
            size_t copy_size = curr_size < new_size ? curr_size : new_size;

            memmove(new_buffer, buffer, copy_size);
            return new_buffer;
        }
    } else {
        assert(0 && "Memory is out of bound of the buffer in this arena");
        return NULL;
    }
}

void
arena_init(Arena* arena, void* buffer, size_t buffer_size)
{
    arena->buffer      = (unsigned char*)buffer;
    arena->buffer_size = buffer_size;
    arena->offset      = 0;
    arena->prev_offset = 0;
}

void
arena_reset(Arena* arena)
{
    arena->offset      = 0;
    arena->prev_offset = 0;
}
void
arena_free(Arena* arena)
{
    arena->offset      = 0;
    arena->prev_offset = 0;

#        ifdef DEBUG
    memset(arena->buffer, 0xDE, arena->buffer_size);
#        endif  // DEBUG
}

ArenaTemp
arena_savepoint_create(Arena* arena)
{
    ArenaTemp temp;

    temp.arena       = arena;
    temp.offset      = arena->offset;
    temp.prev_offset = arena->prev_offset;
    return temp;
}

void
arena_savepoint_restore(ArenaTemp temp_arena)
{
    temp_arena.arena->offset      = temp_arena.offset;
    temp_arena.arena->prev_offset = temp_arena.prev_offset;
}

#    endif  // ARENA_IMPLEMENTATION

#endif
