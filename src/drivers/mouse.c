#include "drivers/mouse.h"

#define MS_DATA_PORT 0x60
#define MS_COMMAND_PORT 0x64

static uint8_t buffer[3];
static uint8_t offset;
static uint8_t buttons;

void mouse_init(void)
{
    offset = 0;
    buttons = 0;

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

    serial_write_string(COM1_BASE_ADDRESS, "[MS] Mouse activated\n", 22);
}

uint32_t mouse_handler(uint32_t esp)
{

    uint8_t status = inb(MS_COMMAND_PORT);
    if (!(status & 0x20)) {
        return esp;
    }

    static int8_t x = 0;
    static int8_t y = 0;

    buffer[offset] = inb(MS_DATA_PORT);
    offset = (offset + 1) % 3;

    if (offset == 0) {
        static uint16_t *video_memory = (uint16_t *)0xB8000;

        video_memory[(80 * y) + x] = (video_memory[(80 * y) + x] & 0xF000) >> 4 |
                                     (video_memory[(80 * y) + x] & 0x0F00) << 4 |
                                     (video_memory[(80 * y) + x] & 0x00FF);

        x = (int8_t)(x + buffer[1]);

        if (x < 0) {
            x = 0;
        }
        if (x >= 80) {
            x = 79;
        }

        y = (int8_t)(y - buffer[2]);
        if (y < 0) {
            y = 0;
        }
        if (y >= 25) {
            y = 24;
        }

        video_memory[(80 * y) + x] = (video_memory[(80 * y) + x] & 0xF000) >> 4 |
                                     (video_memory[(80 * y) + x] & 0x0F00) << 4 |
                                     (video_memory[(80 * y) + x] & 0x00FF);
    }

    for (uint8_t i = 0; i < 3; i++) {
        if ((buffer[0] & (0x01 << i)) != (buttons & (0x01 << i))) {
            /* TODO: CREATE ALL THE MOUSE CLICK LOGICS*/
        }
    }
    buttons = buffer[0];

    return esp;
}