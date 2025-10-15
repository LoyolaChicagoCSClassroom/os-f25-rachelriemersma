#include <stdint.h>
#include <stddef.h>
#include "rprintf.h"
#include "interrupt.h"
#include "page.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

void putc(int data);

void main() {
    remap_pic();
    load_gdt();
    init_idt();
    
    // Initialize page frame allocator
    init_pfa_list();
    esp_printf(putc, "Page frame allocator initialized\r\n");
    
    // Test allocating 2 pages
    struct ppage *pages = allocate_physical_pages(2);
    if (pages != NULL) {
        esp_printf(putc, "Allocated 2 pages successfully!\r\n");
        esp_printf(putc, "Page 1 physical addr: 0x%x\r\n", pages->physical_addr);
        esp_printf(putc, "Page 2 physical addr: 0x%x\r\n", pages->next->physical_addr);
        
        // Free the pages
        free_physical_pages(pages);
        esp_printf(putc, "Freed pages successfully!\r\n");
    } else {
        esp_printf(putc, "Failed to allocate pages!\r\n");
    }
    
    // Test keyboard
    esp_printf(putc, "\r\nKeyboard test - start typing:\r\n");
    
    asm("sti");
    
    while(1) {
        asm("hlt");
    }
}