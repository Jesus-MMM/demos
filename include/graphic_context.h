/* graphic_context.h - Contexto grafico abstraido sobre el driver VGA.
   Proporciona la interfaz de dibujo (put_pixel, fill_rectangle) que usa
   el sistema de widgets, desacoplada del hardware subyacente. */

#pragma once

#include "drivers/vga.h"
#include "types.h"

/** graphic_context_t - Contexto grafico del sistema.
   Encapsula el driver VGA mediante punteros a funciones de dibujo. */
typedef struct graphic_context {
    void (*put_pixel)(uint32_t x, uint32_t y, uint8_t color);
    void (*fill_rectangle)(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t color);
} graphic_context_t;

/** graphic_context_init - Inicializa un contexto grafico con los handlers
   por defecto del driver VGA.
   @gc: contexto grafico a inicializar */
void graphic_context_init(graphic_context_t *gc);

/** graphic_context_set_current - Establece el contexto grafico activo. */
void graphic_context_set_current(graphic_context_t *gc);

/** graphic_context_get_current - Devuelve el contexto grafico activo. */
graphic_context_t *graphic_context_get_current(void);

/** graphic_context_put_pixel - Dibuja un pixel indexado de 8 bits.
   @gc:    contexto grafico
   @x,y:   coordenadas del pixel
   @color: indice de color de la paleta VGA */
void graphic_context_put_pixel(graphic_context_t *gc, uint32_t x, uint32_t y, uint8_t color);

/** graphic_context_fill_rectangle - Rellena un rectangulo con un color.
   @gc:    contexto grafico
   @x,y:   esquina superior izquierda
   @w,h:   ancho y alto
   @color: indice de color de la paleta VGA */
void graphic_context_fill_rectangle(graphic_context_t *gc, uint32_t x, uint32_t y, uint32_t w,
                                    uint32_t h, uint8_t color);

/** graphic_context_flush - Copia el back buffer completo a la pantalla.
   Debe llamarse al final de cada frame, despues de haber dibujado todo el
   contenido (widgets + cursor) sobre el contexto. Espera al retrace vertical
   para evitar el parpadeo y el tearing de la imagen.
   @gc: contexto grafico a vaciar */
void graphic_context_flush(graphic_context_t *gc);
