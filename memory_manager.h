/*Petter Eriksson, 2024-10-04, git: Milloz-dev*, peer22@student.bth.se*/
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct Mblock {
    void* ptr;               // Pointer to the memory allocated
    size_t size;                // Size of the block
    struct Mblock *next;   // Pointer to the next block
    int is_free;                // Is this block free? (1 for true, 0 for false)
} Mblock;

//declare functions
void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t size);
void mem_deinit();

#endif // MEMORY_MANAGER_H
