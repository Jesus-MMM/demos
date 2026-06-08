#include "splash.h"
#include "big_text.h"
#include "timer.h"
#include "io.h"

#define N_LETTERS 5
#define TXT_W (N_LETTERS * CHAR_W + (N_LETTERS - 1))
#define TXT_H CHAR_H
#define PAD 3
#define BIN_W (TXT_W + PAD * 2)
#define BIN_H (TXT_H + 2)
#define B_W (BIN_W + 2)
#define B_H (BIN_H + 2)
#define B_C ((80 - B_W) / 2)
#define B_R ((25 - B_H) / 2)
#define T_C (B_C + 1 + PAD)
#define T_R (B_R + 1 + 1)

void animate_splash(void)
{
    const uint8_t(*glyphs[N_LETTERS])[CHAR_W] = {glyph_D, glyph_e, glyph_m, glyph_O, glyph_S};

    draw_box(B_C, B_R, B_W, B_H, GREEN);

    for (uint16_t i = 0; i < N_LETTERS; i++)
    {
        uint16_t lc = T_C + i * (CHAR_W + 1);

        draw_big_char(glyphs[i], T_R, lc, DARKGREY);
        delay(30000000);

        draw_big_char(glyphs[i], T_R, lc, GREEN);
        delay(15000000);

        draw_big_char(glyphs[i], T_R, lc, LIGHTGREEN);
        delay(15000000);
    }

    delay(60000000);

    for (uint16_t p = 0; p < 2; p++)
    {
        for (uint16_t i = 0; i < N_LETTERS; i++)
        {
            uint16_t lc = T_C + i * (CHAR_W + 1);
            draw_big_char(glyphs[i], T_R, lc, GREEN);
        }
        delay(40000000);

        for (uint16_t i = 0; i < N_LETTERS; i++)
        {
            uint16_t lc = T_C + i * (CHAR_W + 1);
            draw_big_char(glyphs[i], T_R, lc, LIGHTGREEN);
        }
        delay(40000000);
    }

    for (uint16_t i = 0; i < N_LETTERS; i++)
    {
        uint16_t lc = T_C + i * (CHAR_W + 1);
        draw_big_char(glyphs[i], T_R, lc, LIGHTGREEN);
    }
}
