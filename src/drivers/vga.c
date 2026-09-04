#include "drivers/vga.h"
#include "asm.h"
#include "drivers/vga_legacy.h"

void init_vga()
{
}
void vga_write_registers(uint8_t *registers)
{
    // misc
    outb(MISC_PORT, *(registers)++);

    // sequencer
    for (uint8_t i = 0; i < 5; i++) {
        outb(SEQUENCER_INDEX_PORT, i);
        outb(SEQUENCER_DATA_PORT, *(registers++));
    }

    // crtc
    outb(CRTC_INDEX_PORT, 0x03);
    outb(CRTC_DATA_PORT, inb(CRTC_DATA_PORT) | 0x80);
    outb(CRTC_INDEX_PORT, 0x11);
    outb(CRTC_DATA_PORT, inb(CRTC_DATA_PORT) & ~0x80);

    for (uint8_t i = 0; i < 25; i++) {
        outb(CRTC_INDEX_PORT, i);
        outb(CRTC_DATA_PORT, *(registers++));
    }

    // gc
    for (uint8_t i = 0; i < 9; i++) {
        outb(GRAFIC_CONTROLLER_INDEX_PORT, i);
        outb(GRAFIC_CONTROLLER_DATA_PORT, *(registers++));
    }

    // AC
    for (uint8_t i = 0; i < 21; i++) {
        inb(ATTRIBUTE_CONTROLLER_RESET_PORT);
        outb(ATTRIBUTE_CONTROLLER_INDEX_PORT, i);
        outb(ATTRIBUTE_CONTROLLER_WRITE_PORT, *(registers++));
    }

    inb(ATTRIBUTE_CONTROLLER_RESET_PORT);
    outb(ATTRIBUTE_CONTROLLER_INDEX_PORT, 0x20);
}

uint8_t *vga_get_framebuffer_segment()
{

    // NOT FORGET cached_segment = NULL; WHEN ADD MORE MODES OR USING TEXT MODE
    static uint8_t *cached_segment;

    if (cached_segment != NULL) {
        return cached_segment;
    }

    outb(GRAFIC_CONTROLLER_INDEX_PORT, 0x06);
    uint8_t segment_number = ((inb(GRAFIC_CONTROLLER_DATA_PORT) >> 2) & 0x03);

    switch (segment_number) {
    default:
    case 0:
        cached_segment = (uint8_t *)0x00000;
        break;
    case 1:
        cached_segment = (uint8_t *)0xA0000;
        break;
    case 2:
        cached_segment = (uint8_t *)0xB0000;
        break;
    case 3:
        cached_segment = (uint8_t *)0xB8000;
        break;
    }

    return cached_segment;
}

bool vga_support_mode(uint32_t width, uint32_t height, uint32_t color_depth)
{
    return width == 320 && height == 200 && color_depth == 8;
}

bool vga_set_mode(uint32_t width, uint32_t height, uint32_t color_depth)
{

    if (!vga_support_mode(width, height, color_depth)) {
        return 0;
    }

    unsigned char mode_320_200_256[] = {
        /* MISC
         */
        0x63,
        /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00};

    vga_write_registers(mode_320_200_256);

    return 1;
};

/* Lee la paleta activa del DAC (256 entradas de 6 bits: R,G,B en 0..63).
   En el arranque, con el modo 320x200x8 recien activado, devuelve la paleta
   VGA por defecto. */
void vga_read_palette(uint8_t (*dac)[3])
{
    for (uint16_t i = 0; i < 256; i++) {
        outb(DAC_READ_INDEX_PORT, (uint8_t)i);
        dac[i][0] = inb(DAC_DATA_PORT);
        dac[i][1] = inb(DAC_DATA_PORT);
        dac[i][2] = inb(DAC_DATA_PORT);
    }
}

void vga_write_pixel(int32_t x, int32_t y, uint8_t color)
{
    if (x < 0 || 320 <= x || y < 0 || 200 <= y) {
        return;
    }
    uint8_t *pixel_address = vga_get_framebuffer_segment() + (320 * y) + x;
    *pixel_address = color;
}

void vga_fill_rectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t color)
{
    for (int32_t Y = y; Y < y + (int32_t)h; Y++) {
        for (int32_t X = x; X < x + (int32_t)w; X++) {
            vga_write_pixel(X, Y, color);
        }
    }
}
