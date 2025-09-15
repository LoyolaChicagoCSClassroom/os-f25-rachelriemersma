#include <stdint.h>
#include "rprintf.h"

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int cursor_x =0;
static int cursor_y = 0;

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
		if (cursor_x >= VGA_WIDTH) {
			cursor_x = 0;
			cursor_y++;
		}
		if (cursor_y >= VGA_HEIGHT) {
			cursor_y = 0;
		}
}
