
#include <stdint.h>

#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6



const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void putc(int data);

void main() {
    unsigned short *vram = (unsigned short*)0xb8000; // Base address of video mem
    const unsigned char color = 7; // gray text on black background

    esp_printf(putc, "Current execution level: Kernel Mode\r\n");
    esp_printf(putc, "Terminal driver working!\r\n");	
    
    for(int i = 1; i <= 30; i++) {
	esp_printf(putc, "Line%d - Test scroll\r\n", i);	
    }

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }
}
