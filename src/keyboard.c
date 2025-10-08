#include <stdint.h>
#include "rprintf.h"

extern void putc(int data);
extern void outb(uint16_t port, uint8_t val);

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`',   0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/',   0,
    '*',
    0,
    ' ',
    0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,
    0,
    0,
    0,
    0,
    0,
    '-',
    0,
    0,
    0,
    '+',
    0,
    0,
    0,
    0,
    0,
    0,   0,   0,
    0,
    0,
    0,
};

uint8_t inb(uint16_t port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void keyboard_handler_main(void) {
    uint8_t scancode;
    
    scancode = inb(0x60);
    
    if (scancode < 128) {
        char key = keyboard_map[scancode];
        
        if (key != 0) {
            putc(key);
        }
    }
    
    outb(0x20, 0x20);
}
