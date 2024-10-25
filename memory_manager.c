/*Petter Eriksson, 2024-10-04, git: Milloz-dev*, peer22@student.bth.se*/
#include "memory_manager.h"

pthread_mutex_t memory_lock;

static char* heap = NULL;        // Pointer to the actual memory pool (data)
static struct Mblock* heap_header = NULL;  // Pointer to the first memory block's metadata

/*
*Initializes the memory manager with a specified size of memory pool. The memory pool could be any data structure, for instance, 
*a large array or a similar contuguous block of memory.
*(You do not have to interact directly with the hardware or the operating system’s memory management functions).
*/
void mem_init(size_t size) {

    pthread_mutex_init(&memory_lock, NULL);

    // Allocate a large contiguous block of memory
    heap = (char*)malloc(size);
    if (heap == NULL) {
        printf("Failed to initialize memory pool.\n");
        exit(1);
    }

    // Allocate memory for the metadata (header) separately
    heap_header = (Mblock*)malloc(sizeof(Mblock));
    if (heap_header == NULL) {
        printf("Failed to initialize memory headers.\n");
        free(heap);
        exit(1);
    }

    // Set the initial block header (outside the pool)
    heap_header->ptr = heap;    // Set pointer to the start of the memory pool
    heap_header->size = size;   // Full size available for allocation
    heap_header->is_free = 1;   
    heap_header->next = NULL;
}

/*Allocation function
*
*Allocates a block of memory of the specified size. Find a suitable block in the pool, mark it as allocated,
*and return the pointer to the start of the allocated block.
*/
void* mem_alloc(size_t size) {

    pthread_mutex_lock(&memory_lock);

    // Start with the first block
    Mblock* current = heap_header;

    // First-fit allocation strategy: looks for the first suitable block 
    while (current != NULL) {
        // Check if the block is free and large enough
        if (current->is_free && current->size >= size) {
            // Mark the block as not free (allocated)
            current->is_free = 0;
            // Check if the current block can be split into a smaller block
            if (current->size > size) {
                // Allocate a new block structure for the remaining memory
                Mblock* new_block = (Mblock*)malloc(sizeof(Mblock));  // Allocate memory for the new block metadata
                if (new_block == NULL) {
                    printf("Failed to allocate memory for new block header.\n");
                    // Unlock before return Error
                    pthread_mutex_unlock(&memory_lock);
                    return NULL;
                }

                // Initialize the new block with the remaining memory details
                new_block->ptr = (char *)current->ptr + size;  // New block starts after the allocated block
                new_block->size = current->size - size;  // Remaining size
                new_block->is_free = 1;  // New block is free
                new_block->next = current->next;  // Link it to the next block

                // Update the properties of the current block to reflect the allocation
                current->size = size;   // Set size to requested size
                current->next = new_block;  // Link new block after the current one
            }
            // Unlock mutex before return
            pthread_mutex_unlock(&memory_lock);

           // Return a pointer to the allocated memory (data part)
            return current->ptr;
        }

        // Move to the next block
        current = current->next;
    }

    // If no suitable block is found
    printf("Error: No suitable memory block for allocation of size %zu bytes.\n", size);

    // Unlock mutex before return
    pthread_mutex_unlock(&memory_lock);
    return NULL;
}

/*Deallocation function
*
*Frees the specified block of memory. For allocation and deallocation, you need a way to track which parts of the memory pool
* are free and which are allocated.
*/
void mem_free(void* block) {
    // Check if the provided pointer is NULL, nothing to free
    if (block == NULL) {
        return;  // Return early since there's nothing to free
    }

    pthread_mutex_lock(&memory_lock);

    // Start searching from the beginning of the memory pool
    struct Mblock* current = heap_header;
    struct Mblock* previous = NULL;

    // Iterate through the memory blocks to find the one to free
    while (current != NULL) {
        // Check if this block corresponds to the provided pointer
        if (current->ptr == block) {
            // Check if block is already free
            if (current->is_free) {
                printf("Warning: Attempt to free already free block at %p.\n", block);
                pthread_mutex_unlock(&memory_lock);
                return;
            }

            current->is_free = 1; // Mark current block as free

            // Check if the next block is free and can be coalesced(ihopsatt)
            if (current->next != NULL && current->next->is_free == 1) {
                struct Mblock* next = current->next; // Next block
                current->size += next->size; // Increase size by the size of the next block
                current->next = next->next; // Bypass the next block
                free(next);
            }
            
            // Check if the previous block is free and can be coalesced(ihopsatt)
            if (previous != NULL && previous->is_free == 1) {
                previous->size += current->size;
                previous->next = current->next;
                free(current);
            }

            // Unlock mutex before exit
            pthread_mutex_unlock(&memory_lock);
            return; // Exit the function
        }
        previous = current;
        current = current->next; // Move to the next block
    }
    // Unlock mutex if no block where found to free
    pthread_mutex_unlock(&memory_lock);
    // If no block was found, print error message
    printf("Error: Freeing a block that was not allocated at %p.\n", block);
}

/*Resize function
*
*Changes the size of the memory block, possibly moving it.
*/
void* mem_resize(void* block, size_t size) {
    // If the provided block is NULL, allocate a new block of the specified size
    if (block == NULL) {
        return mem_alloc(size);  // New allocation
    }

    pthread_mutex_lock(&memory_lock);

    // Find the corresponding header for the block
    Mblock* header = heap_header; // Start from the beginning
    while (header) {
        if (header->ptr == block) { // Compare with ptr directly
            break;  // Found the corresponding header
        }
        header = header->next; // Move to next block header
    }

    // If the header is not found or the current block is large enough, return the original block
    if (!header || header->size >= size) {
        // Unlock mutex before return
        pthread_mutex_unlock(&memory_lock);
        return block;
    }

    pthread_mutex_unlock(&memory_lock);

    // Allocate a new block of the requested size
    void* new_block = mem_alloc(size);

    pthread_mutex_lock(&memory_lock);

    if (new_block == NULL) {
        pthread_mutex_unlock(&memory_lock);
        return NULL;  // Allocation failed
    }

    // Copy the old data to the new block and free the old block
    size_t copy_size = header->size < size ? header->size : size; // Copy only what fits
    memcpy(new_block, block, copy_size); // Use memcpy to copy data

    pthread_mutex_unlock(&memory_lock);

    // Free the old block
    mem_free(block);

    return new_block; // Return the pointer to the new block
}

/*Deinit function
*
*Frees up the memory pool that was initially allocated by the mem_init function,
*ensuring that all allocated memory is returned to the system.
*/
void mem_deinit() {

    pthread_mutex_lock(&memory_lock);
    // Check if the memory pool has already been deinitialized
    if (heap == NULL) {
        printf("Memory pool is already deinitialized.\n");
        // Unlock mutex before return
        pthread_mutex_unlock(&memory_lock);
    return; // Early return since there's nothing to deinitialize
    }

    // Free all headers
    Mblock* current = heap_header; // Start from first
    while (current != NULL) {
        Mblock* next = current->next; // Store the pointer to the next block header
        free(current);  // Free the memory allocated for the current block header
        current = next; // Move to the next block header
    }
    heap_header = NULL; // Reset the header pointer to NULL after freeing all headers

    // Free the main memory pool
    free(heap);
    heap = NULL; // Set pointer to NULL to avoid dangling references

    // Unlock mutex when deinit done
    pthread_mutex_unlock(&memory_lock);
}
