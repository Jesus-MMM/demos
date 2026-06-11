/* io.c - Implementacion de operaciones de E/S sobre el framebuffer VGA
   y el controlador CRTC para modo texto 80x25. */

#include "io.h"
#include "asm.h"

/** write_letter_to_buffer - Escribe un caracter con atributo de color en una
 * posicion especifica del framebuffer VGA.
 * @letter: codigo ASCII del caracter a mostrar
 * @row: fila destino (0-24)
 * @col: columna destino (0-79)
 * @fg_color: color de primer plano (4 bits, 0-15)
 * @bg_color: color de fondo (4 bits, 0-15)
 *
 * El framebuffer VGA en modo texto color tiene una entrada de 16 bits
 * por celda: byte bajo = ASCII, byte alto = atributo (bg << 4 | fg). */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void write_letter_to_buffer(uint8_t letter, uint16_t row, uint16_t col, uint8_t fg_color,
                            uint8_t bg_color)
{
    volatile uint16_t *framebuffer = (volatile uint16_t *)FRAMEBUFFER;

    uint16_t attribute = (bg_color << 4) | (fg_color & 0x0F);
    uint16_t character_with_attribute = (attribute << 8) | (letter & 0x00FF);

    uint16_t position = (row * 80) + col;
    // NOLINTNEXTLINE(clang-analyzer-core.FixedAddressDereference)
    framebuffer[position] = character_with_attribute;
}

/** move_cursor - Actualiza la posicion del cursor del hardware VGA.
 * @pos: posicion lineal (fila * 80 + columna) a la que mover el cursor.
 *
 * El cursor se controla escribiendo en los puertos CRTC: primero el
 * byte alto, luego el byte bajo de la posicion. */
void move_cursor(uint16_t pos)
{
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

/** scroll - Desplaza el viewport de pantalla hacia arriba configurando
 * el registro de inicio de pantalla del CRTC.
 * @row: fila que se convierte en la nueva fila 0 del viewport. */
void scroll(uint16_t row)
{
    uint16_t pos = 80 * row;
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, SCREEN_START_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, SCREEN_START_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}

/** write_letter_to_screen - Escribe un caracter en la fila superior (fila 0).
 * @c: caracter a escribir
 * @pos: posicion lineal (columna) en la fila 0. */
void write_letter_to_screen(const char c, uint16_t pos)
{
    write_letter_to_buffer(c, 0, pos, WHITE, BLACK);
}

/** write_to_screen - Escribe una cadena de caracteres en la fila superior
 * y posiciona el cursor al final.
 * @buf: puntero a los caracteres a escribir
 * @len: cantidad de caracteres */
void write_to_screen(const char *buf, uint16_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        write_letter_to_buffer(buf[i], 0, i, WHITE, BLACK);
    }
    move_cursor(len);
}

/** print_byte - Imprime la representacion binaria (8 bits) de un byte.
 * @pbyte: puntero al byte a imprimir
 * @pos: posicion lineal inicial en la fila 0 */
void print_byte(const uint8_t *pbyte, uint32_t pos)
{
    for (int16_t bit = 0; bit < 8; bit++) {
        uint8_t mask = (uint8_t)0x1 << (7 - bit);
        if (*pbyte & mask) {
            write_letter_to_screen('1', pos + bit);
        } else {
            write_letter_to_screen('0', pos + bit);
        }
    }
}

/** style_cursor - Configura el estilo del cursor de hardware VGA.
 * @cstyle: estilo deseado:
 *   - BIG:     cursor cuadrado (de 0 a 15)
 *   - SMALL:   cursor linea fina (de 12 a 15)
 *   - DISABLE: cursor oculto (bit 5 del registro de inicio)
 *   - ENABLE:  cursor visible (bit 5 despejado) */
void style_cursor(CursorStyle cstyle)
{
    uint8_t start;
    switch (cstyle) {
    case BIG:
        outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
        outb(CRTC_DATA_PORT, 0x00);
        break;

    case SMALL:
        outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
        outb(CRTC_DATA_PORT, 0x0C);
        break;

    case DISABLE:
        outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
        start = inb(CRTC_DATA_PORT);
        outb(CRTC_DATA_PORT, start | 0x20);
        break;

    case ENABLE:
        outb(CRTC_CMD_PORT, CURSOR_STYLE_START_CMD);
        start = inb(CRTC_DATA_PORT);
        outb(CRTC_DATA_PORT, start & 0xBF);
        break;

    default:
    }
}
