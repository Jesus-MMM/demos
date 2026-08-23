/* vga.h - Controlador de pantalla VGA y registro de cursor.
   Proporciona funciones para escribir en el framebuffer VGA en modo texto
   de 80x25 y controlar el cursor via puertos CRTC. */

#pragma once

#include "asm.h"
#include "types.h"

/* Direccion del framebuffer VGA en modo texto color */
#define FRAMEBUFFER 0x000B8000

/* Puertos del controlador CRTC (Cathode Ray Tube Controller) */
#define CRTC_CMD_PORT 0x3D4
#define CRTC_DATA_PORT 0x3D5

/* Comandos de posicion de cursor */
#define CURSOR_POS_HIGH_BYTE_CMD 0x0E
#define CURSOR_POS_LOW_BYTE_CMD 0x0F

/* Comandos de estilo de cursor */
#define CURSOR_STYLE_START_CMD 0x0A
#define CURSOR_STYLE_END_CMD 0x0B

/* Comandos de posicion de inicio de pantalla (scroll) */
#define SCREEN_START_POS_HIGH_BYTE_CMD 0x0C
#define SCREEN_START_POS_LOW_BYTE_CMD 0x0D

/* Colores VGA estandar (4 bits: 0-15) */
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define MAGENTA 0x5
#define BROWN 0x6
#define LIGHTGREY 0x7
#define DARKGREY 0x8
#define LIGHTBLUE 0x9
#define LIGHTGREEN 0xA
#define LIGHTCYAN 0xB
#define LIGHTRED 0xC
#define LIGHTMAGENTA 0xD
#define LIGHTBROWN 0xE
#define WHITE 0xF

typedef enum { BIG, SMALL, DISABLE, ENABLE } CursorStyle;

/** move_cursor - Mueve el cursor del hardware VGA a una posicion lineal.
 * @pos: posicion lineal en el buffer (fila * 80 + columna) */
void move_cursor(uint16_t pos);

/** scroll - Desplaza el viewport de pantalla a una fila especifica.
 * @row: numero de fila que se convierte en la nueva fila superior */
void scroll(uint16_t row);

/** write_letter_to_buffer - Escribe un caracter con atributos en el framebuffer VGA.
 * @letter: codigo ASCII del caracter
 * @row, @col: posicion en la grilla de 80x25
 * @fg_color: color de primer plano (4 bits)
 * @bg_color: color de fondo (4 bits) */
void write_letter_to_buffer(uint8_t letter, uint16_t row, uint16_t col, uint8_t fg_color,
                            uint8_t bg_color);

/** write_letter_to_screen - Escribe un caracter en la fila superior (0).
 * @c: caracter a escribir
 * @pos: posicion lineal en la fila */
void write_letter_to_screen(char c, uint16_t pos);

/** write_to_screen - Escribe una cadena en la fila superior y mueve el cursor.
 * @buf: puntero a la cadena (no necesariamente terminada en nulo)
 * @len: longitud de la cadena */
void write_to_screen(const char *buf, uint16_t len);

/** print_byte - Imprime los 8 bits de un byte como '1' y '0' en pantalla.
 * @pbyte: puntero al byte a imprimir
 * @pos: posicion lineal inicial en la fila 0 */
void print_byte(const uint8_t *pbyte, uint32_t pos);

/** style_cursor - Cambia el estilo (tamano/visibilidad) del cursor de hardware.
 * @cstyle: estilo deseado (BIG, SMALL, DISABLE, ENABLE) */
void style_cursor(CursorStyle cstyle);
