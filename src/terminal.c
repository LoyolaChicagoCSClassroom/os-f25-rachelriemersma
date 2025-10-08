#include <stdint.h>
#include "rprintf.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int cursor_x = 0;
static int cursor_y = 0;

static void scroll_screen() {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;

    // Move each line up by one
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (0x07 << 8) | ' ';
    }

    cursor_y = VGA_HEIGHT - 1;
}

void putc(int data) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
    char c = (char)data;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c >= 32) {
        vga[cursor_y * VGA_WIDTH + cursor_x] = (0x07 << 8) | c;
        cursor_x++;
    }

    // Wrap horizontally
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // Scroll if we’ve reached bottom
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }
}
