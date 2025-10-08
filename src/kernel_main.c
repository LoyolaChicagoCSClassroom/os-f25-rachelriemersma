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

    esp_printf(putc, "Polling keyboard for scancodes...\r\n\r\n");

    while (1) {
        keyboard_poll_once();
        for (volatile int i = 0; i < 50000; i++) { }  // small pause
    }
}
