#include <stdint.h>
#include "rprintf.h"

extern void putc(int data);  

uint8_t inb(uint16_t port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(rv) : "dN"(port));
    return rv;
}

void keyboard_poll_once(void) {
    uint8_t status = inb(0x64);          // Step 1: read status
    if (status & 0x01) {                 // Step 2: LSB set?
        uint8_t sc = inb(0x60);          // Step 3: read scancode
        esp_printf(putc, "Scancode: 0x%02x\r\n", sc);  // Step 4: print
    }
}
