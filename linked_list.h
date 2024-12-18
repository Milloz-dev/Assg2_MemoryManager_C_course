/*Petter Eriksson, 2024-10-04, git: Milloz-dev*, peer22@student.bth.se*/
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdint.h>
#include "memory_manager.h"
#include <pthread.h>

// Node structure 
typedef struct Node {
    uint16_t data;
    struct Node* next;
} Node;

// Declare a mutex for thread safety
extern pthread_mutex_t list_lock;  // Declare it as extern

// Declare functions
void list_init(Node** head, size_t size);
void list_insert(Node** head, uint16_t data);
void list_insert_after(Node* prev_node, uint16_t data);
void list_insert_before(Node** head, Node* next_node, uint16_t data);
void list_delete(Node** head, uint16_t data);
Node* list_search(Node** head, uint16_t data);
void list_display(Node** head);
void list_display_range(Node** head, Node* start_node, Node* end_node);
int list_count_nodes(Node** head);
void list_cleanup(Node** head);

#endif // LINKED_LIST_H
