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
    
    esp_printf(putc, "Keyboard Driver Initialized\r\n");
    esp_printf(putc, "Start typing...\r\n\r\n");
    
    asm("sti");
    
    while(1) {
        asm("hlt");
    }
}
