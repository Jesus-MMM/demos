/* window.h - Ventana: un widget compuesto con barra de titulo que puede
   arrastrarse con el raton y recibir el foco. */

#pragma once

#include "gui/widget.h"
#include "types.h"

/** window_t - Ventana de la interfaz grafica.
   Es un composite_widget_t con una etiqueta de texto y estado de arrastre. */
typedef struct {
    composite_widget_t base;

    char *label;
    bool dragging;
} window_t;

/** window_init - Inicializa una ventana.
   @parent: widget padre (normalmente el desktop)
   @x,y,w,h: posicion y tamaño
   @color: color de fondo (indice de paleta VGA)
   @label: titulo de la ventana (NULL si no tiene) */
void window_init(window_t *win, widget_t *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                 uint8_t color, char *label);

/** window_draw - Dibuja la ventana y su barra de titulo. */
void window_draw(window_t *win, graphic_context_t *gc);

/** window_on_mouse_down - Inicia el arrastre si se pulsa con el boton
   izquierdo; pide el foco. */
void window_on_mouse_down(window_t *win, int32_t x, int32_t y, uint8_t button);

/** window_on_mouse_up - Termina el arrastre. */
void window_on_mouse_up(window_t *win, int32_t x, int32_t y, uint8_t button);

/** window_on_mouse_move - Mueve la ventana si esta siendo arrastrada. */
void window_on_mouse_move(window_t *win, int32_t oldx, int32_t oldy, int32_t newx, int32_t newy);
