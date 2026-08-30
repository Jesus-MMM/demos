/* mouse.c - Driver del raton PS/2 para kernel i386.
   Puerto de datos: 0x60
   Puerto de comando: 0x64
   Decodifica paquetes de 3 bytes y notifica movimientos/clics
   a traves del callback on_mouse_move registrado. */

#include "drivers/mouse.h"

static mouse_driver_t *active_mouse = NULL;

static void mouse_activate(driver_t *self)
{
    mouse_driver_t *ms = (mouse_driver_t *)self;

    ms->offset = 0;
    ms->buttons = 0;
    ms->x = 40;
    ms->y = 12;

    while (inb(MS_COMMAND_PORT) & 0x1) {
        inb(MS_DATA_PORT);
    }

    outb(MS_COMMAND_PORT, 0xA8);

    outb(MS_COMMAND_PORT, 0x20);
    while (!(inb(MS_COMMAND_PORT) & 0x1)) {
    }
    uint8_t status = (inb(MS_DATA_PORT) | 2);
    outb(MS_COMMAND_PORT, 0x60);
    while (inb(MS_COMMAND_PORT) & 0x2) {
    }
    outb(MS_DATA_PORT, status);

    outb(MS_COMMAND_PORT, 0xD4);
    while (inb(MS_COMMAND_PORT) & 0x2) {
    }
    outb(MS_DATA_PORT, 0xF4);

    active_mouse = ms;

    serial_write_string(COM1_BASE_ADDRESS, "[MS] Mouse activated\n", 22);
}

static int mouse_reset(driver_t *self)
{
    (void)self;
    return 0;
}

static void mouse_deactivate(driver_t *self)
{
    (void)self;
    active_mouse = NULL;
}

static uint32_t mouse_handle_interrupt(driver_t *self, uint32_t esp)
{
    mouse_driver_t *ms = (mouse_driver_t *)self;

    uint8_t status = inb(MS_COMMAND_PORT);
    if (!(status & 0x20)) {
        return esp;
    }

    ms->buffer[ms->offset] = inb(MS_DATA_PORT);
    ms->offset = (ms->offset + 1) % 3;

    if (ms->offset == 0) {
        if (ms->on_mouse_move) {
            ms->on_mouse_move((int8_t)ms->buffer[1], (int8_t)ms->buffer[2], ms->handler_data);
        }
    }

    for (uint8_t i = 0; i < 3; i++) {
        if ((ms->buffer[0] & (0x01 << i)) != (ms->buttons & (0x01 << i))) {
            if (ms->on_mouse_button) {
                bool pressed = (ms->buffer[0] & (0x01 << i)) != 0;
                ms->on_mouse_button(i, ms->x, ms->y, pressed, ms->handler_data);
            }
        }
    }
    ms->buttons = ms->buffer[0];

    return esp;
}

void mouse_driver_init(mouse_driver_t *drv, on_mouse_move_fn on_move, void *data)
{
    drv->base.name = "mouse";
    drv->base.irq = 0x2C;
    drv->base.activate = mouse_activate;
    drv->base.reset = mouse_reset;
    drv->base.deactivate = mouse_deactivate;
    drv->base.handle_interrupt = mouse_handle_interrupt;

    drv->offset = 0;
    drv->buttons = 0;
    drv->x = 40;
    drv->y = 12;
    drv->on_mouse_move = on_move;
    drv->on_mouse_button = NULL;
    drv->handler_data = data ? data : drv;
}

/* --- Handler por defecto: cursor VGA XOR --- */

void mouse_default_on_move(int8_t x_offset, int8_t y_offset, void *data)
{
    mouse_driver_t *ms = (mouse_driver_t *)data;
    static uint16_t *video_memory = (uint16_t *)0xB8000;

    video_memory[(80 * ms->y) + ms->x] = (video_memory[(80 * ms->y) + ms->x] & 0xF000) >> 4 |
                                         (video_memory[(80 * ms->y) + ms->x] & 0x0F00) << 4 |
                                         (video_memory[(80 * ms->y) + ms->x] & 0x00FF);

    ms->x = (int8_t)(ms->x + x_offset);
    if (ms->x < 0) {
        ms->x = 0;
    }
    if (ms->x >= 80) {
        ms->x = 79;
    }

    ms->y = (int8_t)(ms->y - y_offset);
    if (ms->y < 0) {
        ms->y = 0;
    }
    if (ms->y >= 25) {
        ms->y = 24;
    }

    video_memory[(80 * ms->y) + ms->x] = (video_memory[(80 * ms->y) + ms->x] & 0xF000) >> 4 |
                                         (video_memory[(80 * ms->y) + ms->x] & 0x0F00) << 4 |
                                         (video_memory[(80 * ms->y) + ms->x] & 0x00FF);
}
