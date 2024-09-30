#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define HEADER_SIZE sizeof(Mblock)

typedef struct Mblock {
    size_t size;
    bool is_free;
    struct Mblock* next;
} Mblock;

static Mblock* heap = NULL; // Pointer to the start of the memory pool

//For this task, you are expected to create a basic memory manager in C, which allows for dynamic allocation, deallocation, and resizing of memory blocks.
//You are expected to implement the following interface / functions:

//Initialization function
void mem_init(size_t size) {
//Initializes the memory manager with a specified size of memory pool. The memory pool could be any data structure, for instance, 
//a large array or a similar contuguous block of memory. (You do not have to interact directly with the hardware or the operating system’s memory management functions).

    // Allocate a large contiguous block of memory
    heap = (Mblock*)malloc(size);
    if (heap == NULL) {
        printf("Failed to initialize memory pool.\n");
        exit(1);
    }

    // Initialize the memory pool as a single large free block
    heap->size = size - HEADER_SIZE;
    heap->is_free = true;
    heap->next = NULL;
}

//Allocation function
void* mem_alloc(size_t size) {
//Allocates a block of memory of the specified size. Find a suitable block in the pool, mark it as allocated, and return the pointer to the start of the allocated block.
    //Size check
    if (size == 0) {
        printf("Error: Cannot allocate zero bytes.\n");
        return NULL;
    }

    Mblock* current = heap;
    
    // First-fit allocation strategy.
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Mark the block as allocated
            current->is_free = false;
            
            // If there's enough space left after allocation, split the block
            if (current->size > size + HEADER_SIZE) {
                Mblock* new_block = (Mblock*)((char*)current + HEADER_SIZE + size);
                new_block->size = current->size - size - HEADER_SIZE;
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            return (char*)current + HEADER_SIZE;  // Return the pointer to the allocated memory block
        }

        current = current->next;
    }

    // If no suitable block is found
    printf("Error: No suitable memory block for allocation of size %zu bytes.\n", size);
    return NULL;
}

//Deallocation function
void mem_free(void* block) {
//Frees the specified block of memory. For allocation and deallocation, you need a way to track which parts of the memory pool are free and which are allocated.
    
    if (block == NULL) {
        printf("Error: Attempted to free a NULL pointer.\n");
        return;  // Return early since there's nothing to free
    }
    
    Mblock* header = (Mblock*)((char*)block - HEADER_SIZE);

    // Check if the block is already free
    if (header->is_free) {
        printf("Error: Double free detected for block at %p.\n", block);
        return;
    }

    header->is_free = true;

    // Coalesce adjacent free blocks
    Mblock* current = heap;
    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            current->size += HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

//Resize function
void* mem_resize(void* block, size_t size) {
//Changes the size of the memory block, possibly moving it.
    if (block == NULL) {
        return mem_alloc(size);  // If block is NULL, behave like mem_alloc
    }

    Mblock* header = (Mblock*)((char*)block - HEADER_SIZE);
    
    if (header->size >= size) {
        // If the current block is big enough, no need to move
        return block;
    }

    // Allocate a new block of the requested size
    void* new_block = mem_alloc(size);
    if (new_block == NULL) {
        return NULL;  // Allocation failed
    }

    // Copy the old data to the new block and free the old block
    memcpy(new_block, block, header->size);
    mem_free(block);

    return new_block;
}

//Deinit function
void mem_deinit() {
//Frees up the memory pool that was initially allocated by the mem_init function, ensuring that all allocated memory is returned to the system.
    free(heap);
    heap = NULL;
}
