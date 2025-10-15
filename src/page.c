#include "page.h"
#include <stddef.h>

#define PAGE_SIZE 0x200000  // 2 MB pages

// Array of 128 pages covering 256 MB of memory
struct ppage physical_page_array[128];

// Pointer to head of free pages list
struct ppage *free_physical_pages = NULL;

// Initialize the free page list
void init_pfa_list(void) {
    // Start of physical memory (after kernel)
    void *physical_start = (void *)0x10000000;  // Starting at 256 MB
    
    // Initialize the free list as empty
    free_physical_pages = NULL;
    
    // Loop through each page and add to free list
    for (int i = 0; i < 128; i++) {
        // Set physical address for this page
        physical_page_array[i].physical_addr = physical_start + (i * PAGE_SIZE);
        
        // Link into free list at the beginning
        physical_page_array[i].next = free_physical_pages;
        physical_page_array[i].prev = NULL;
        
        if (free_physical_pages != NULL) {
            free_physical_pages->prev = &physical_page_array[i];
        }
        
        free_physical_pages = &physical_page_array[i];
    }
}

// Allocate npages from the free list
struct ppage *allocate_physical_pages(unsigned int npages) {
    if (npages == 0 || free_physical_pages == NULL) {
        return NULL;
    }
    
    struct ppage *allocated_list = NULL;
    struct ppage *current;
    
    // Allocate the requested number of pages
    for (unsigned int i = 0; i < npages; i++) {
        if (free_physical_pages == NULL) {
            // Not enough pages available
            // Return what we allocated back to free list
            if (allocated_list != NULL) {
                free_physical_pages(allocated_list);
            }
            return NULL;
        }
        
        // Remove first page from free list
        current = free_physical_pages;
        free_physical_pages = free_physical_pages->next;
        
        if (free_physical_pages != NULL) {
            free_physical_pages->prev = NULL;
        }
        
        // Add to allocated list
        current->next = allocated_list;
        current->prev = NULL;
        
        if (allocated_list != NULL) {
            allocated_list->prev = current;
        }
        
        allocated_list = current;
    }
    
    return allocated_list;
}

// Free pages back to the free list
void free_physical_pages(struct ppage *ppage_list) {
    if (ppage_list == NULL) {
        return;
    }
    
    struct ppage *current = ppage_list;
    struct ppage *next;
    
    // Walk through the list being freed
    while (current != NULL) {
        next = current->next;
        
        // Add this page back to free list
        current->next = free_physical_pages;
        current->prev = NULL;
        
        if (free_physical_pages != NULL) {
            free_physical_pages->prev = current;
        }
        
        free_physical_pages = current;
        current = next;
    }
}