#include "graphic_context.h"
#include "asm.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

/* Puerto del controlador de atributos: lectura aqui da el estado del
   vertical retrace en el bit 3 (1 = retrace en curso). */
#define VGA_STATUS_PORT 0x3DA
#define VGA_STATUS_VRETRACE 0x08

static const graphic_context_t *current_gc = NULL;

/* Back buffer: se renderiza el frame completo en memoria y solo se copia a
   la pantalla cuando termina (graphic_context_flush). Evita que el frame a
   medio dibujar sea visible, eliminando el parpadeo. */
static uint8_t back_buffer[SCREEN_BUFFER_SIZE];

static void default_put_pixel(uint32_t x, uint32_t y, uint8_t color)
{
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }
    back_buffer[(SCREEN_WIDTH * y) + x] = color;
}

static void default_fill_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t color)
{
    for (uint32_t Y = y; Y < y + h; Y++) {
        for (uint32_t X = x; X < x + w; X++) {
            default_put_pixel(X, Y, color);
        }
    }
}

/** Espera el cierre del retrace vertical para copiar el frame sin tearing. */
static void wait_for_vsync(void)
{
    /* Si hay un retrace en curso, esperar a que termine. */
    while ((inb(VGA_STATUS_PORT) & VGA_STATUS_VRETRACE) != 0) {
    }
    /* Esperar al siguiente inicio de retrace (periodo fuera de retrace). */
    while ((inb(VGA_STATUS_PORT) & VGA_STATUS_VRETRACE) == 0) {
    }
    /* Dejar pasar un tick para copiar durante el retrace. */
    while ((inb(VGA_STATUS_PORT) & VGA_STATUS_VRETRACE) != 0) {
    }
}

void graphic_context_init(graphic_context_t *gc)
{
    gc->put_pixel = default_put_pixel;
    gc->fill_rectangle = default_fill_rectangle;
}

void graphic_context_set_current(graphic_context_t *gc)
{
    current_gc = gc;
}

graphic_context_t *graphic_context_get_current(void)
{
    return (graphic_context_t *)current_gc;
}

void graphic_context_put_pixel(graphic_context_t *gc, uint32_t x, uint32_t y, uint8_t color)
{
    if (gc->put_pixel != NULL) {
        gc->put_pixel(x, y, color);
    }
}

void graphic_context_fill_rectangle(graphic_context_t *gc, uint32_t x, uint32_t y, uint32_t w,
                                    uint32_t h, uint8_t color)
{
    if (gc->fill_rectangle != NULL) {
        gc->fill_rectangle(x, y, w, h, color);
    }
}

void graphic_context_blit_image(graphic_context_t *gc, uint32_t x, uint32_t y, uint32_t w,
                                uint32_t h, const uint8_t *pixels)
{
    (void)gc;
    if (pixels == NULL || w == 0 || h == 0) {
        return;
    }
    /* El caller garantiza que la imagen cabe en pantalla desde (x,y); se
       copian w bytes por fila al back buffer, fila a fila. */
    for (uint32_t row = 0; row < h; row++) {
        uint32_t src = row * w;
        uint32_t dst = ((y + row) * SCREEN_WIDTH) + x;
        for (uint32_t col = 0; col < w; col++) {
            back_buffer[dst + col] = pixels[src + col];
        }
    }
}

void graphic_context_flush(graphic_context_t *gc)
{
    (void)gc;

    wait_for_vsync();

    uint32_t *dst = (uint32_t *)vga_get_framebuffer_segment();
    uint32_t *src = (uint32_t *)back_buffer;
    for (uint32_t i = 0; i < SCREEN_BUFFER_SIZE / 4; i++) {
        dst[i] = src[i];
    }
}
