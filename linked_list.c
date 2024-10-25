/*Petter Eriksson, 2024-10-04, git: Milloz-dev*, peer22@student.bth.se*/
#include "linked_list.h"

pthread_mutex_t list_lock;

/*Initialization function
*
*This function sets up the list and prepares it for operations.
*/
void list_init(Node** head, size_t size) {

    pthread_mutex_lock(&list_lock);

    // Initialize the memory manager with the specified size of memory pool
    mem_init(size);
    *head = NULL;  // Initialize the list head to NULL (empty list)

    // Unlock after init
    pthread_mutex_unlock(&list_lock);
}

/*Insertion function(s)
*
*Adds a new node with the specified data to the linked list. The new node will be added after the last node,
*known as inserting at the rear end. Feel free to extend this function to allow insertions at any position.
*/
void list_insert(Node** head, uint16_t data) {
    // Lock list to prevent other threads from inserting or deleting nodes
    pthread_mutex_lock(&list_lock);

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory alloc for new node failed.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }
    // Initialize the new node's data and next pointer
    new_node->data = data; // Assign the provided data to the new node
    new_node->next = NULL;

    // Check if the linked list is empty
    if (*head == NULL) {
        // List is empty, new node becomes the head
        *head = new_node;
    } else {
        // Traverse the list to find the last node
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next; // Move to the next node
        }
        // Append the new node at the end of the list
        current->next = new_node;  // Link the last node to the new node
    }
    // Unlock list after insertion
    pthread_mutex_unlock(&list_lock);
}

//Inserts a new node with the specified data immediately after a given node.
void list_insert_after(Node* prev_node, uint16_t data){
    // Lock list to prevent other threads from inserting or deleting nodes
    pthread_mutex_lock(&list_lock);

    // Check if the previous node is NULL, if true exits
    if (prev_node == NULL) {
        printf("Error: Previous node cannot be NULL.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation for new node failed.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    // Initialize the new node's data and next pointer
    new_node->data = data;
    new_node->next = prev_node->next;

    // Insert the new node into the list after the previous node
    prev_node->next = new_node;

    // Unlock list after insertion
    pthread_mutex_unlock(&list_lock);
}

//Inserts a new node with the specified data immediately before a given node in the list. A bit trickier than list_insert_after method.
//You have to consider cases when the next_node is the head, you need to find the previous node (note this is a single linked list), ...
void list_insert_before(Node** head, Node* next_node, uint16_t data){
    // Lock list to prevent other threads from inserting or deleting nodes
    pthread_mutex_lock(&list_lock);

    // Check if the next node is NULL or if the list is empty
    if (next_node == NULL || *head == NULL) {
        printf("Error: Invalid node or empty list.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    // Allocate memory for the new node
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Error: Memory allocation for new node failed.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }
 
    new_node->data = data; // Initialize the new node's data

    // Special case: Insert before the head node
    if (*head == next_node) {
        new_node->next = *head; // Link new node to current head
        *head = new_node; // Update head to the new node
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    // Find the node just before the next_node
    Node* current = *head;
    while (current != NULL && current->next != next_node) {
        current = current->next;
    }

    // Check if the previous node was found
    if (current == NULL) {
        printf("Error: next_node not found in the list.\n");
        mem_free(new_node); // Free allocated memory for new node
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    // Link the new node in the list
    new_node->next = current->next;  // Link new node to the next node
    current->next = new_node;         // Link the previous node to the new node

    // Unlock list after insertion
    pthread_mutex_unlock(&list_lock);
}

/*Deletion function
*
*Removes a node with the specified data from the linked list.
*/
void list_delete(Node** head, uint16_t data) {
    // Lock list to prevent other threads from inserting or deleting nodes
    pthread_mutex_lock(&list_lock);

    // Check if the list is empty
    if (*head == NULL) {
        printf("Error: Cannot delete from an empty list.\n");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }

    Node* current = *head; // Start from the head node
    Node* prev = NULL; // Initialize a pointer to track the previous node

    while (current != NULL) {
        // If the node to be deleted is the head
        if (prev == NULL && current->data == data) {
            *head = current->next; // Update head to point to the next node
            mem_free(current); // Free the memory of the old head node
            pthread_mutex_unlock(&list_lock); // Unlock before return
            return;
        }

        // If current node's data matches the target
        if (current->data == data) {
            prev->next = current->next; // Bypass the node to be deleted
            mem_free(current); // Free the memory of the node
            pthread_mutex_unlock(&list_lock); // Unlock after deletion
            return;
        }

        // Update previous and current pointers
        prev = current; // Track the previous node
        current = current->next; // Move to the next node
    }

    // Node not found
    printf("Error: Node with data %u not found.\n", data);
    
    // Unlock list
    pthread_mutex_unlock(&list_lock);
}

/*Search function
*
*Searches for a node with the specified data and returns a pointer to it.
*/
Node* list_search(Node** head, uint16_t data){
    // Lock the list to prevent modifications from other threads
    pthread_mutex_lock(&list_lock);

    Node* current = *head; // Start searching from the head node

    // Traverse the list until the end is reached
    while (current != NULL) {
        // Check if the current node's data matches the target data
        if (current->data == data) {
            // Unlock before returning found node
            pthread_mutex_unlock(&list_lock);
            return current;  // Node found; return a pointer to it
        }
        current = current->next; // Move to the next node
    }
    // Unlock before return NULL
    pthread_mutex_unlock(&list_lock);
    return NULL;  // Node not found
}

/*Display function(s)
*
*Prints all the elements in the linked list. The expected output format is to display each element of the
*linked list separated by commas, enclosed in square brackets. For example [10, 20, 30, 40, ...]
*/
void list_display(Node** head){
    list_display_range(head, NULL, NULL);
}

/*Prints all elements of the list between two nodes (start_node and end_node). If start_node is NULL, it should start from the beginning. 
*If end_node is NULL, it should print until the end. I.e., list_display_range(&head, NULL, NULL) should print all elements of the list.
*Ranges are inclusive, i.e. list_display_range(&head, 5, 7) in a linked list [1, 2, 3, 4, 5, 6, 7, 8, 9] should print [5, 6, 7]*/
void list_display_range(Node** head, Node* start_node, Node* end_node){
    // Lock the list to prevent modifications from other threads
    pthread_mutex_lock(&list_lock);

    // Check if the head of the list is NULL, indicating the list is empty
    if (head == NULL || *head == NULL) {
        printf("[]");
        // Unlock before return
        pthread_mutex_unlock(&list_lock);
        return;
    }
    
    // If start_node is NULL, start from the head of the list
    if (start_node == NULL) {
        start_node = *head;
    }

    printf("["); // Start the output with an opening bracket

    // Traverse the list from start_node to end_node
    while (start_node != NULL) {
        printf("%d", start_node->data); // Print the current node's data
        
        // Stop if we've reached the end_node
        if (start_node == end_node) {
            break;
        }

        // Check if there is a next node to print
        if (start_node->next != NULL) {
            printf(", ");
        }
        
        start_node = start_node->next; // Move to the next node
    }

    printf("]"); // Close the output with a closing bracket

    // Unlock after displaying
    pthread_mutex_unlock(&list_lock);
}

/*Nodes count function
*
*Will simply return the count of nodes. You can either keep track of the number of nodes in a variable 
*(which would require incrementing or decrementing everytime a new node is added, respectively deleted),
*or iterate over all nodes starting from the head and count them until it reaches the end of the list.
*/
int list_count_nodes(Node** head){
    // Lock the list to prevent modifications from other threads
    pthread_mutex_lock(&list_lock);

    int count = 0; // Initialize a counter
    Node* current = *head;// Start with the head of the list
    // Traverse the list until the end (NULL) is reached
    while (current != NULL) {
        count++; // Increment the counter for each node
        current = current->next; // Move to the next node
    }

    // Unlock list after counting
    pthread_mutex_unlock(&list_lock); 
    return count;
}

/*Cleanup function
*
*Frees all the nodes in the linked list. Important to prevent memory leaks.
*/
void list_cleanup(Node** head){
    // Lock the list to prevent modifications from other threads
    pthread_mutex_lock(&list_lock);

    Node* current = *head; // Start with the head of the list
    Node* next = NULL; // Pointer to store the next node

    // Traverse the list and free each node's memory
    while (current != NULL) {
        next = current->next; // Save the next node before freeing the current
        mem_free(current);
        current = next; // Next
    }

    *head = NULL;  // Reset the head pointer to NULL after cleanup
    mem_deinit();
    // Unlock list after cleanup
    pthread_mutex_unlock(&list_lock); 
}