#include "paging.h"
#include <stdint.h>
#include <stddef.h>

struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page pt[1024] __attribute__((aligned(4096)));

void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    uint32_t virt_addr = (uint32_t)vaddr;
    struct ppage *current = pglist;
    
    while (current != NULL) {
        uint32_t pd_index = virt_addr >> 22;
        
        uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
        
        if (!pd[pd_index].present) {
            pd[pd_index].present = 1;
            pd[pd_index].rw = 1;
            pd[pd_index].user = 0;
            pd[pd_index].frame = ((uint32_t)pt) >> 12;
        }
        
        pt[pt_index].present = 1;
        pt[pt_index].rw = 1;
        pt[pt_index].user = 0;
        pt[pt_index].frame = ((uint32_t)current->physical_addr) >> 12;
        
        virt_addr += 4096;
        current = current->next;
    }
    
    return vaddr;
}

void loadPageDirectory(struct page_directory_entry *pd) {
    asm("mov %0,%%cr3" : : "r"(pd));
}

void enable_paging(void) {
    asm("mov %cr0, %eax\n"
        "or $0x80000001,%eax\n"
        "mov %eax,%cr0");
}