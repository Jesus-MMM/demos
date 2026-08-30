/* desktop.h - Escritorio: widget compuesto raiz a pantalla completa que
   recibe los eventos del raton y teclado (via callbacks registrados en los
   drivers) y los despacha a sus ventanas. Tambien dibuja el cursor. */

#pragma once

#include "graphic_context.h"
#include "gui/widget.h"
#include "types.h"

/** desktop_t - Escritorio de la interfaz grafica.
   Guarda la posicion del cursor y las dimensiones de pantalla. */
typedef struct {
    composite_widget_t base;

    int32_t mouse_x;
    int32_t mouse_y;
    int32_t width;
    int32_t height;
} desktop_t;

/** desktop_init - Inicializa el escritorio a pantalla completa.
   @desktop: escritorio a inicializar
   @gc: contexto grafico (para conocer la resolucion de pantalla) */
void desktop_init(desktop_t *desktop, graphic_context_t *gc);

/** desktop_draw - Dibuja el escritorio, sus hijos y el cursor del raton. */
void desktop_draw(desktop_t *desktop, graphic_context_t *gc);

/* --- Callbacks que se registran en los drivers --- */

/** desktop_on_mouse_move - Actualiza la posicion del cursor y despacha el
   movimiento a las ventanas. Firma compatible con on_mouse_move_fn.
   @data: puntero al desktop_t */
void desktop_on_mouse_move(int8_t x_offset, int8_t y_offset, void *data);

/** desktop_on_mouse_button - Despacha un click (presionar o soltar) a la
   ventana bajo el cursor. Firma compatible con on_mouse_button_fn.
   @data: puntero al desktop_t */
void desktop_on_mouse_button(uint8_t button, int8_t x, int8_t y, bool pressed, void *data);

/** desktop_on_key_down - Despacha una tecla presionada al hijo enfocado.
   Firma compatible con on_key_down_fn.
   @data: puntero al desktop_t */
void desktop_on_key_down(char c, void *data);

/** desktop_on_key_up - Despacha una tecla soltada al hijo enfocado.
   Firma compatible con on_key_up_fn.
   @data: puntero al desktop_t */
void desktop_on_key_up(char c, void *data);
