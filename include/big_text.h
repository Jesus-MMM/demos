#pragma once

#include "types.h"

#define CHAR_W 5
#define CHAR_H 5

extern const uint8_t glyph_D[CHAR_H][CHAR_W];
extern const uint8_t glyph_e[CHAR_H][CHAR_W];
extern const uint8_t glyph_m[CHAR_H][CHAR_W];
extern const uint8_t glyph_O[CHAR_H][CHAR_W];
extern const uint8_t glyph_S[CHAR_H][CHAR_W];

void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);

void draw_big_char(const uint8_t (*glyph)[CHAR_W], uint16_t r0, uint16_t c0, uint8_t color);
