/* keyboard.c - Driver del teclado PS/2 para kernel i386.
   Puerto de datos: 0x60
   Puerto de comando: 0x64
   Traduce scancodes Set 1 a caracteres ASCII y los escribe
   en el framebuffer VGA usando las funciones de vga.h. */

#include "drivers/keyboard.h"
#include "asm.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

#define KB_DATA_PORT    0x60
#define KB_COMMAND_PORT 0x64

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

static uint16_t cursor_row = 0;
static uint16_t cursor_col = 0;

void keyboard_set_cursor(uint16_t row, // NOLINT(bugprone-easily-swappable-parameters)
                         uint16_t col) // NOLINT(bugprone-easily-swappable-parameters)
{
    cursor_row = row;
    cursor_col = col;
    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}

static void keyboard_newline(void)
{
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= SCREEN_ROWS) {
        cursor_row = SCREEN_ROWS - 1;
        scroll(1);
    }
    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}

static void keyboard_backspace(void)
{
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = SCREEN_COLS - 1;
    }
    write_letter_to_buffer(' ', cursor_row, cursor_col, WHITE, BLACK);
    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}

static void keyboard_putchar(char c)
{
    if (c == '\n') {
        keyboard_newline();
        return;
    }

    if (c == '\b') {
        keyboard_backspace();
        return;
    }

    write_letter_to_buffer(c, cursor_row, cursor_col, WHITE, BLACK);
    cursor_col++;

    if (cursor_col >= SCREEN_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= SCREEN_ROWS) {
            cursor_row = SCREEN_ROWS - 1;
            scroll(1);
        }
    }

    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}

void keyboard_init(void)
{
    while (inb(KB_COMMAND_PORT) & 0x1) {
        inb(KB_DATA_PORT);
    }

    outb(KB_COMMAND_PORT, 0xAE);

    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_COMMAND_PORT, 0x20);
    while (!(inb(KB_COMMAND_PORT) & 0x1)) {}
    uint8_t status = (inb(KB_DATA_PORT) | 1) & ~0x10;
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_COMMAND_PORT, 0x60);
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_DATA_PORT, status);
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_DATA_PORT, 0xF4);

    serial_write_string(COM1_BASE_ADDRESS, "[KB] Keyboard activated\n", 24);
}

uint32_t keyboard_handler(uint32_t esp)
{
    uint8_t key = inb(KB_DATA_PORT);

    if (key >= 0x80) {
        return esp;
    }

    char c = 0;

    switch (key) {
        case 0x02: c = '1'; break;
        case 0x03: c = '2'; break;
        case 0x04: c = '3'; break;
        case 0x05: c = '4'; break;
        case 0x06: c = '5'; break;
        case 0x07: c = '6'; break;
        case 0x08: c = '7'; break;
        case 0x09: c = '8'; break;
        case 0x0A: c = '9'; break;
        case 0x0B: c = '0'; break;

        case 0x10: c = 'q'; break;
        case 0x11: c = 'w'; break;
        case 0x12: c = 'e'; break;
        case 0x13: c = 'r'; break;
        case 0x14: c = 't'; break;
        case 0x15: c = 'y'; break;
        case 0x16: c = 'u'; break;
        case 0x17: c = 'i'; break;
        case 0x18: c = 'o'; break;
        case 0x19: c = 'p'; break;

        case 0x1E: c = 'a'; break;
        case 0x1F: c = 's'; break;
        case 0x20: c = 'd'; break;
        case 0x21: c = 'f'; break;
        case 0x22: c = 'g'; break;
        case 0x23: c = 'h'; break;
        case 0x24: c = 'j'; break;
        case 0x25: c = 'k'; break;
        case 0x26: c = 'l'; break;

        case 0x27: c = (char)0xF1; break;
        case 0x2C: c = 'z'; break;
        case 0x2D: c = 'x'; break;
        case 0x2E: c = 'c'; break;
        case 0x2F: c = 'v'; break;
        case 0x30: c = 'b'; break;
        case 0x31: c = 'n'; break;
        case 0x32: c = 'm'; break;
        case 0x33: c = ','; break;
        case 0x34: c = '.'; break;
        case 0x35: c = '-'; break;

        case 0x1C: c = '\n'; break;
        case 0x39: c = ' '; break;
        case 0x0E: c = '\b'; break;

        default:
            break;
    }

    if (c != 0) {
        keyboard_putchar(c);
    }

    return esp;
}
