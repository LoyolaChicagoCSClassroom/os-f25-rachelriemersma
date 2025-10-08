#include <stdint.h>
#include "rprintf.h"
#include "interrupt.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

void putc(int data);

void main() {
    remap_pic();
    load_gdt();
    init_idt();
    
    for (int i = 0; i < 100; i++) {
        esp_printf(putc, "This is line %d\n", i);
    }
    
    asm("sti");
    
    while(1) {
        asm("hlt");
    }
}