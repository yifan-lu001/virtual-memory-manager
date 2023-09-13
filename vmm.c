#include "vmm.h"

// Memory Manager implementation
// Implement all other functions here...

page_info *new_page(int page_number, int frame_number, int protection)
{
    page_info *page = malloc(sizeof(page_info));
    page->page_number = page_number;
    page->frame_number = frame_number;
    page->protection = protection;
    return page;
}

// Create a new node
node_t *new_node(page_info *page)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node->page = page;
    node->next = NULL;
    return node;
}

// Create a new queue
queue_t *new_queue(int max_size)
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    return queue;
}

// Add an item to the end of the queue
void enqueue(queue_t *queue, page_info *page)
{
    node_t *node = new_node(page);
    queue->size++;
    if (queue->tail == NULL)
    {
        queue->head = queue->tail = node;
        return;
    }
    queue->tail->next = node;
    queue->tail = node;
}

// Search queue for page, return page or NULL
page_info *get_page(queue_t *queue, int page_number)
{
    node_t *node = queue->head;
    while (node != NULL)
    {
        if (node->page->page_number == page_number)
        {
            return node->page;
        }
        node = node->next;
    }
    return NULL;
}

// Remove an item from the front of the queue
node_t *dequeue(queue_t *queue)
{
    if (queue->head == NULL)
    {
        return NULL;
    }
    node_t *node = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL)
    {
        queue->tail = NULL;
    }
    queue->size--;
    return node;
}

/*
    CIRCULAR QUEUE
*/

// Create a new page for third chance
third_page_info *new_third_page(int page_number, int frame_number, int protection)
{
    third_page_info *page = malloc(sizeof(third_page_info));
    page->page_number = page_number;
    page->frame_number = frame_number;
    page->R = 0;
    page->M = 0;
    page->skipped = 0;
    return page;
}

// New node
circular_node_t *new_circular_node(third_page_info *page)
{
    circular_node_t *node = (circular_node_t *)malloc(sizeof(circular_node_t));
    node->page = page;
    node->next = NULL;
    return node;
}

// Create circular queue
circular_queue_t *new_circular_queue(int max_size)
{
    circular_queue_t *queue = (circular_queue_t *)malloc(sizeof(circular_queue_t));
    queue->head = NULL;
    queue->size = 0;
    queue->max_size = max_size;
    return queue;
}

// Add an item after head
void enqueue_circular(circular_queue_t *queue, third_page_info *page)
{
    circular_node_t *node = new_circular_node(page);

    queue->size++;
    if (queue->head == NULL) // Previously empty
    {
        queue->head = node;
        queue->head->next = queue->head;
    }
    else {
        // Add after current head
        node->next = queue->head->next;
        queue->head->next = node;
        queue->head = node;
    }
}

// Search circular queue for page, return page or NULL
third_page_info *get_page_circular(circular_queue_t *queue, int page_number)
{
    circular_node_t *node = queue->head;
    for (int i=0; i<queue->size; i++)
    {
        if (node->page->page_number == page_number)
        {
            return node->page;
        }
        node = node->next;
    }
    return NULL;
}

// Remove using third-chance algorithm
circular_node_t *remove_page_circular(void *va, int page_size, circular_queue_t *queue)
{
    // Assumes algorithm will work
    while (true)
    {
        third_page_info *page = queue->head->page;
        if (page->R == 0)
        {
            if (page->M == 0 || page->skipped == 1)
            {
                return queue->head;
            }
            page->skipped = 1; // Have now skipped over it once
        }
        else
        {
            page->R = 0; 
            // Protect page
            mprotect(va + page->page_number * page_size, page_size, PROT_NONE);
        }
        queue->head = queue->head->next;
    }
}

// DEBUGGING
void info_dump(circular_queue_t *queue)
{
    circular_node_t *node = queue->head;
    for (int i = 0; i < queue->size; i++)
    {
        printf("Page %d, R %d, M %d, skipped %d \n", node->page->page_number, node->page->R, node->page->M, node->page->skipped);
        node = node->next;
    }
}