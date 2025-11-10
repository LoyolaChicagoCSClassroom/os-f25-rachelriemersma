#include <stdint.h>
#include <stddef.h>
#include "rprintf.h"
#include "interrupt.h"
#include "page.h"
#include "paging.h"
#include "fat.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

void putc(int data);

void main() {
    remap_pic();
    load_gdt();
    init_idt();
    
    init_pfa_list();
    esp_printf(putc, "Page frame allocator initialized\r\n");
    
    struct ppage *pages = allocate_physical_pages(2);
    if (pages != NULL) {
        esp_printf(putc, "Allocated 2 pages successfully!\r\n");
        esp_printf(putc, "Page 1 physical addr: 0x%x\r\n", pages->physical_addr);
        esp_printf(putc, "Page 2 physical addr: 0x%x\r\n", pages->next->physical_addr);
        
        free_physical_pages(pages);
        esp_printf(putc, "Freed pages successfully!\r\n");
    } else {
        esp_printf(putc, "Failed to allocate pages!\r\n");
    }
    
    extern struct page_directory_entry pd[];
    extern char _end_kernel;
    
    esp_printf(putc, "Setting up paging...\r\n");
    
    struct ppage tmp;
    tmp.next = NULL;
    
    for (uint32_t addr = 0x100000; addr < (uint32_t)&_end_kernel; addr += 4096) {
        tmp.physical_addr = (void *)addr;
        map_pages((void *)addr, &tmp, pd);
    }
    
    tmp.physical_addr = (void *)0xB8000;
    map_pages((void *)0xB8000, &tmp, pd);
    
    uint32_t esp;
    asm("mov %%esp,%0" : "=r" (esp));
    uint32_t stack_start = esp & 0xFFFFF000;
    
    for (uint32_t addr = stack_start; addr < stack_start + 0x4000; addr += 4096) {
        tmp.physical_addr = (void *)addr;
        map_pages((void *)addr, &tmp, pd);
    }
    
    loadPageDirectory(pd);
    enable_paging();
    
    esp_printf(putc, "Paging enabled!\r\n");

    esp_printf(putc, "\r\nTesting FAT filesystem...\r\n");

    if (fatInit() == 0) {
        esp_printf(putc, "FAT init successful!\r\n");
        
        
        struct file *f = fatOpen("TEST.TXT");
        if (f != NULL) {
            esp_printf(putc, "Found TEST.TXT, size: %d bytes\r\n", f->rde.file_size);
            
            char buf[512];
            int bytes = fatRead(f, buf, 512);
            buf[bytes] = '\0';  
            
            esp_printf(putc, "File contents:\r\n%s\r\n", buf);
        } else {
            esp_printf(putc, "TEST.TXT not found\r\n");
        }
    }
    
    esp_printf(putc, "\r\nKeyboard test - start typing:\r\n");
    
    asm("sti");
    
    while(1) {
        asm("hlt");
    }
}