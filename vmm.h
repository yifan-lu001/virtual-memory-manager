#ifndef VMM_H
#define VMM_H

#include "interface.h"

// Declare your own data structures and functions here...


// Page info
typedef struct page_info
{
    int page_number;
    int frame_number;
    // protection, ie PROT_NONE or PROT_READ | PROT_WRITE
    int protection;
} page_info;

// Node
typedef struct node
{
    page_info *page;
    struct node *next;
} node_t;

// Queue
typedef struct queue
{
    node_t *head, *tail;
    int size;
    int max_size;
} queue_t;


// Circular linked list implementation
typedef struct third_page_info
{
    int page_number;
    int frame_number;
    int R;
    int M;
    int skipped;
} third_page_info;

typedef struct circular_node
{
    third_page_info *page;
    struct circular_node *next;
} circular_node_t;

typedef struct circular_queue
{
    circular_node_t *head, *tail;
    int size;
    int max_size;
} circular_queue_t;


page_info *new_page(int page_number, int frame_number, int protection);
node_t *new_node(page_info *page);
queue_t *new_queue(int max_size);
void enqueue(queue_t *queue, page_info *page);
node_t *dequeue(queue_t *queue);
page_info *get_page(queue_t *queue, int page_number);

third_page_info *new_third_page(int page_number, int frame_number, int protection);
circular_node_t *new_circular_node(third_page_info *page);
circular_queue_t *new_circular_queue(int max_size);
void enqueue_circular(circular_queue_t *queue, third_page_info *page);
third_page_info *get_page_circular(circular_queue_t *queue, int page_number);
circular_node_t *remove_page_circular(void *va, int page_size, circular_queue_t *queue);
void info_dump(circular_queue_t *queue);

#endif
