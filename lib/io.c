#include "io.h"
#include "asm.h"

void write_letter_to_buffer(uint8_t letter, uint16_t row, uint16_t col, uint8_t fg_color, uint8_t bg_color)
{
    volatile uint16_t *framebuffer = (volatile uint16_t*) FRAMEBUFFER;

    // Formato VGA: byte bajo = carácter ASCII, byte alto = atributo (color)
    // Atributo: bits 4-7 = background, bits 0-3 = foreground
    uint16_t attribute = (bg_color << 4) | (fg_color & 0x0F);
    uint16_t character_with_attribute = (attribute << 8) | (letter & 0x00FF);
    
    // Calcular posición: fila * ancho + columna
    uint16_t position = row * 80 + col;
    framebuffer[position] = character_with_attribute;
}

void move_cursor(uint16_t pos)
{
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

void scroll(uint16_t row)
{
    uint16_t pos = 80*row;
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;
 
    outb(CRTC_CMD_PORT, SCREEN_START_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, SCREEN_START_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

void write_to_screen(const char *buf, uint16_t len)
{
    for (uint32_t i=0; i<len; i++)
    {
        write_letter_to_buffer(buf[i], 0, i, WHITE, BLACK);
    }
    move_cursor(len);
}