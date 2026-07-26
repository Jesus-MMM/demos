/* keyboard.c - Driver del teclado PS/2 para kernel i386.
   Puerto de datos: 0x60
   Puerto de comando: 0x64
   Traduce scancodes Set 1 a caracteres ASCII y los notifica
   a traves del callback on_key_down registrado. */

#include "drivers/keyboard.h"

static keyboard_driver_t *active_keyboard = NULL;

static void keyboard_activate(driver_t *self)
{
    keyboard_driver_t *kb = (keyboard_driver_t *)self;

    while (inb(KB_COMMAND_PORT) & 0x1) {
        inb(KB_DATA_PORT);
    }

    outb(KB_COMMAND_PORT, 0xAE);

    while (inb(KB_COMMAND_PORT) & 0x2) {
    }
    outb(KB_COMMAND_PORT, 0x20);
    while (!(inb(KB_COMMAND_PORT) & 0x1)) {
    }
    uint8_t status = (inb(KB_DATA_PORT) | 1) & ~0x10;
    while (inb(KB_COMMAND_PORT) & 0x2) {
    }
    outb(KB_COMMAND_PORT, 0x60);
    while (inb(KB_COMMAND_PORT) & 0x2) {
    }
    outb(KB_DATA_PORT, status);
    while (inb(KB_COMMAND_PORT) & 0x2) {
    }
    outb(KB_DATA_PORT, 0xF4);

    active_keyboard = kb;

    serial_write_string(COM1_BASE_ADDRESS, "[KB] Keyboard activated\n", 24);
}

static int keyboard_reset(driver_t *self)
{
    (void)self;
    return 0;
}

static void keyboard_deactivate(driver_t *self)
{
    (void)self;
    active_keyboard = NULL;
}

static uint32_t keyboard_handle_interrupt(driver_t *self, uint32_t esp)
{
    keyboard_driver_t *kb = (keyboard_driver_t *)self;
    uint8_t key = inb(KB_DATA_PORT);

    if (key >= 0x80) {
        return esp;
    }

    if (!kb->on_key_down) {
        return esp;
    }

    char c = 0;

    switch (key) {
    case 0x02:
        c = '1';
        break;
    case 0x03:
        c = '2';
        break;
    case 0x04:
        c = '3';
        break;
    case 0x05:
        c = '4';
        break;
    case 0x06:
        c = '5';
        break;
    case 0x07:
        c = '6';
        break;
    case 0x08:
        c = '7';
        break;
    case 0x09:
        c = '8';
        break;
    case 0x0A:
        c = '9';
        break;
    case 0x0B:
        c = '0';
        break;

    case 0x10:
        c = 'q';
        break;
    case 0x11:
        c = 'w';
        break;
    case 0x12:
        c = 'e';
        break;
    case 0x13:
        c = 'r';
        break;
    case 0x14:
        c = 't';
        break;
    case 0x15:
        c = 'y';
        break;
    case 0x16:
        c = 'u';
        break;
    case 0x17:
        c = 'i';
        break;
    case 0x18:
        c = 'o';
        break;
    case 0x19:
        c = 'p';
        break;

    case 0x1E:
        c = 'a';
        break;
    case 0x1F:
        c = 's';
        break;
    case 0x20:
        c = 'd';
        break;
    case 0x21:
        c = 'f';
        break;
    case 0x22:
        c = 'g';
        break;
    case 0x23:
        c = 'h';
        break;
    case 0x24:
        c = 'j';
        break;
    case 0x25:
        c = 'k';
        break;
    case 0x26:
        c = 'l';
        break;

    case 0x27:
        c = (char)0xF1;
        break;
    case 0x2C:
        c = 'z';
        break;
    case 0x2D:
        c = 'x';
        break;
    case 0x2E:
        c = 'c';
        break;
    case 0x2F:
        c = 'v';
        break;
    case 0x30:
        c = 'b';
        break;
    case 0x31:
        c = 'n';
        break;
    case 0x32:
        c = 'm';
        break;
    case 0x33:
        c = ',';
        break;
    case 0x34:
        c = '.';
        break;
    case 0x35:
        c = '-';
        break;

    case 0x1C:
        c = '\n';
        break;
    case 0x39:
        c = ' ';
        break;
    case 0x0E:
        c = '\b';
        break;

    default:
        break;
    }

    if (c != 0) {
        kb->on_key_down(c, kb->handler_data);
    }

    return esp;
}

void keyboard_driver_init(keyboard_driver_t *drv, on_key_down_fn on_key_down_fn, void *handler_data)
{
    drv->base.name = "keyboard";
    drv->base.irq = 0x21;
    drv->base.activate = keyboard_activate;
    drv->base.reset = keyboard_reset;
    drv->base.deactivate = keyboard_deactivate;
    drv->base.handle_interrupt = keyboard_handle_interrupt;

    drv->cursor_row = 0;
    drv->cursor_col = 0;
    drv->on_key_down = on_key_down_fn;
    drv->on_key_up = NULL;
    drv->handler_data = handler_data ? handler_data : drv;
}

/* --- Handler por defecto: escribe en VGA --- */

static void kb_newline(keyboard_driver_t *kb)
{
    kb->cursor_col = 0;
    kb->cursor_row++;
    if (kb->cursor_row >= SCREEN_ROWS) {
        kb->cursor_row = SCREEN_ROWS - 1;
        scroll(1);
    }
    move_cursor((kb->cursor_row * SCREEN_COLS) + kb->cursor_col);
}

static void kb_backspace(keyboard_driver_t *kb)
{
    if (kb->cursor_col > 0) {
        kb->cursor_col--;
    } else if (kb->cursor_row > 0) {
        kb->cursor_row--;
        kb->cursor_col = SCREEN_COLS - 1;
    }
    write_letter_to_buffer(' ', kb->cursor_row, kb->cursor_col, WHITE, BLACK);
    move_cursor((kb->cursor_row * SCREEN_COLS) + kb->cursor_col);
}

void keyboard_default_on_key_down(char c, void *data)
{
    keyboard_driver_t *kb = (keyboard_driver_t *)data;

    if (c == '\n') {
        kb_newline(kb);
        return;
    }

    if (c == '\b') {
        kb_backspace(kb);
        return;
    }

    write_letter_to_buffer(c, kb->cursor_row, kb->cursor_col, WHITE, BLACK);
    kb->cursor_col++;

    if (kb->cursor_col >= SCREEN_COLS) {
        kb->cursor_col = 0;
        kb->cursor_row++;
        if (kb->cursor_row >= SCREEN_ROWS) {
            kb->cursor_row = SCREEN_ROWS - 1;
            scroll(1);
        }
    }

    move_cursor((kb->cursor_row * SCREEN_COLS) + kb->cursor_col);
}

void keyboard_set_cursor(uint16_t row, uint16_t col)
{
    if (active_keyboard) {
        active_keyboard->cursor_row = row;
        active_keyboard->cursor_col = col;
    }
    move_cursor((row * SCREEN_COLS) + col);
}
