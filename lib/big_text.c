#include "big_text.h"
#include "io.h"

const uint8_t glyph_D[CHAR_H][CHAR_W] = {
    {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0},
};

const uint8_t glyph_e[CHAR_H][CHAR_W] = {
    {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1},
};

const uint8_t glyph_m[CHAR_H][CHAR_W] = {
    {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},
};

const uint8_t glyph_O[CHAR_H][CHAR_W] = {
    {1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 1},
};

const uint8_t glyph_S[CHAR_H][CHAR_W] = {
    {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 1},
};

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color)
{
    uint16_t r;
    uint16_t c;
    uint16_t rt = x + w - 1;
    uint16_t bt = y + h - 1;

    write_letter_to_buffer(0xC9, y, x, color, BLACK);
    for (c = x + 1; c < rt; c++) {
        write_letter_to_buffer(0xCD, y, c, color, BLACK);
    }
    write_letter_to_buffer(0xBB, y, rt, color, BLACK);

    for (r = y + 1; r < bt; r++) {
        write_letter_to_buffer(0xBA, r, x, color, BLACK);
        for (c = x + 1; c < rt; c++) {
            write_letter_to_buffer(' ', r, c, color, BLACK);
        }
        write_letter_to_buffer(0xBA, r, rt, color, BLACK);
    }

    write_letter_to_buffer(0xC8, bt, x, color, BLACK);
    for (c = x + 1; c < rt; c++) {
        write_letter_to_buffer(0xCD, bt, c, color, BLACK);
    }
    write_letter_to_buffer(0xBC, bt, rt, color, BLACK);
}

void draw_big_char(const uint8_t (*glyph)[CHAR_W], uint16_t r0, uint16_t c0, uint8_t color)
{
    for (uint16_t r = 0; r < CHAR_H; r++) {
        for (uint16_t c = 0; c < CHAR_W; c++) {
            write_letter_to_buffer(glyph[r][c] ? 0xDB : ' ', r0 + r, c0 + c, color, BLACK);
        }
    }
}
