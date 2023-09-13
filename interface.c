#include "interface.h"
#include "vmm.h"

// Interface implementation
// Implement APIs here...

// Global variables
queue_t *queue;
circular_queue_t *circ_queue;
void *va;
int page_size;

void mm_init(enum policy_type policy, void *vm, int vm_size, int num_frames, int page_sizeLoc)
{
    // initialize global variables
    queue = new_queue(num_frames);
    circ_queue = new_circular_queue(num_frames);
    va = vm;
    page_size = page_sizeLoc;

    // protect the virtual memory
    mprotect(va, vm_size, PROT_NONE);

    // initialize signal handler
    struct sigaction handler;

    handler.sa_sigaction = (policy == MM_FIFO) ? handler_fifo : handler_third_chance;
    handler.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &handler, NULL);
    return;
}

// Get physical address
int get_physical_address(int frame_number, int offset)
{
    int physical_address = frame_number * page_size + offset;
    return physical_address;
}

// Catch SIGSEGV signal using sigaction
void handler_fifo(int signum, siginfo_t *info, void *ucontext)
{
    // Get the address that caused the SIGSEGV
    void *addr = info->si_addr;
    int page_num = (addr - va) / page_size;
    int offset = (addr - va) % page_size;

    // Get error
    int error = ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_ERR]; // Is there a better way?
    int write = (error & 0x2) / 2;

    // Determine correct protection from error
    int protection = (write) ? (PROT_WRITE | PROT_READ) : PROT_READ;

    // Initialize logging variables
    int wb = 0;
    int fault_type;
    int frame_number;
    int evicted_page_num = -1;

    // Check if the page is in the queue
    page_info *search = get_page(queue, page_num);
    if (!search)
    {
        fault_type = write;

        // If the queue is full, evict the first item
        if (queue->size >= queue->max_size)
        {
            // Evict the first item
            page_info *evicted = dequeue(queue)->page;

            // Update write back
            wb = (evicted->protection == (PROT_READ | PROT_WRITE));

            evicted_page_num = evicted->page_number;
            frame_number = evicted->frame_number;
            // Protect the evicted page
            mprotect(va + evicted_page_num * page_size, page_size, PROT_NONE);
        }
        else
        {
            frame_number = queue->size;
        }

        // If the page is not in the queue, add the page to the queue
        page_info *page = new_page(page_num, frame_number, protection);
        enqueue(queue, page);
    }
    else
    {
        frame_number = search->frame_number;
        fault_type = 2;
        search->protection = protection;
    }
    // Protect the page
    mprotect(va + page_num * page_size, page_size, protection);

    int physical_addr = get_physical_address(frame_number, offset);
    mm_logger(page_num, fault_type, evicted_page_num, wb, physical_addr);
}

void handler_third_chance(int signum, siginfo_t *info, void *ucontext)
{
    // Get the address that caused the SIGSEGV
    void *addr = info->si_addr;
    int page_num = (addr - va) / page_size;
    int offset = (addr - va) % page_size;

    // Get error
    int error = ((ucontext_t *)ucontext)->uc_mcontext.gregs[REG_ERR]; // Is there a better way?
    int write = (error & 0x2) / 2;

    // Initialize logging variables
    int wb = 0;
    int fault_type;
    int frame_number;
    int protection = (write) ? (PROT_WRITE | PROT_READ) : PROT_READ;
    int evicted_page_num = -1;

    // Check if the page is in the queue
    // printf("virtual_page=%d\n", page_num);
    third_page_info *search = get_page_circular(circ_queue, page_num);
    if (!search)
    {
        // Set fault type
        fault_type = write;

        // Initialize page
        third_page_info *page = new_third_page(page_num, frame_number, protection);
        page->R = 1;
        page->M = write;

        // If the queue is full, evict
        if (circ_queue->size >= circ_queue->max_size)
        {
            circ_queue->head=circ_queue->head->next; // Advance the head
    
            // Evict the item
            circular_node_t *evicted = remove_page_circular(va, page_size, circ_queue);
            third_page_info *evicted_page = evicted->page;

            //Update frame number
            frame_number = evicted_page->frame_number;
            page->frame_number = frame_number;

            // Update write back
            wb = (evicted_page->M == 1);

            // Protect the evicted page
            evicted_page_num = evicted_page->page_number;
            mprotect(va + evicted_page_num * page_size, page_size, PROT_NONE);

            // Replace the evicted page with new page
            evicted->page = page;
        }
        else // Not yet full
        {
            frame_number = circ_queue->size;
            page->frame_number = frame_number;
            // Add to queue
            enqueue_circular(circ_queue, page);
        }
    }
    else
    {
        // Update extra bits
        search->R = 1;
        search->skipped=0;

        // Update frme number
        frame_number = search->frame_number;

        // Determine fault type and update M bit if needed
        if (write)
        {
            fault_type = (search->M) ? 4 : 2;
            search->M = 1;
        }
        else {
            fault_type = 3;
        }
    }
    // Protect the page 
    mprotect(va + page_num * page_size, page_size, protection);

    int physical_addr = get_physical_address(frame_number, offset);
    mm_logger(page_num, fault_type, evicted_page_num, wb, physical_addr);
}
