/* big_text.h - Definiciones para renderizado de caracteres grandes (5x5).
   Cada glifo es una matriz de 5x5 donde 1 = encendido, 0 = apagado. */

#pragma once

#include "types.h"

#define CHAR_W 5
#define CHAR_H 5

extern const uint8_t glyph_D[CHAR_H][CHAR_W];
extern const uint8_t glyph_e[CHAR_H][CHAR_W];
extern const uint8_t glyph_m[CHAR_H][CHAR_W];
extern const uint8_t glyph_O[CHAR_H][CHAR_W];
extern const uint8_t glyph_S[CHAR_H][CHAR_W];

/** draw_box - Dibuja un recuadro con caracteres de linea del juego de caracteres VGA.
 * @x, @y: coordenada superior izquierda (columna, fila)
 * @w, @h: ancho y alto del recuadro
 * @color: color del borde */
void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
              uint8_t color); // NOLINT(bugprone-easily-swappable-parameters)

/** draw_big_char - Dibuja un caracter grande en pantalla usando una matriz glifo.
 * @glyph: matriz 5x5 que define la forma del caracter
 * @r0, @c0: posicion en pantalla (fila, columna) de la esquina superior izquierda
 * @color: color del primer plano */
void draw_big_char(const uint8_t (*glyph)[CHAR_W], uint16_t r0, uint16_t c0, uint8_t color);
