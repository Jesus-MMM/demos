#pragma once

#include "asm.h"
#include <types.h>

#define MISC_PORT 0x3C2
#define CRTC_INDEX_PORT 0x3D4
#define CRTC_DATA_PORT 0x3D5
#define SEQUENCER_INDEX_PORT 0x3C4
#define SEQUENCER_DATA_PORT 0x3C5
#define GRAFIC_CONTROLLER_INDEX_PORT 0x3CE
#define GRAFIC_CONTROLLER_DATA_PORT 0x3CF
#define ATTRIBUTE_CONTROLLER_INDEX_PORT 0x3C0
#define ATTRIBUTE_CONTROLLER_READ_PORT 0x3C1
#define ATTRIBUTE_CONTROLLER_WRITE_PORT 0x3C0
#define ATTRIBUTE_CONTROLLER_RESET_PORT 0x3DA
#define DAC_READ_INDEX_PORT 0x3C7
#define DAC_DATA_PORT 0x3C9

void init_vga();
void vga_write_registers(uint8_t *registers);

uint8_t *vga_get_framebuffer_segment();
bool vga_support_mode(uint32_t width, uint32_t height, uint32_t color_depth);
bool vga_set_mode(uint32_t width, uint32_t height, uint32_t color_depth);

void vga_write_pixel(int32_t x, int32_t y, uint8_t color);
void vga_fill_rectangle(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t color);
void vga_read_palette(uint8_t (*dac)[3]);
