/* widget.h - Sistema de widgets del kernel. Define un widget_t base con posicion,
   tamaño, color y eventos de entrada, y un composite_widget_t que contiene
   hijos y gestiona el foco. */

#pragma once

#include "graphic_context.h"
#include "types.h"

#define WIDGET_CHILDREN_CAPACITY 100

typedef struct widget widget_t;

/** Tipos de puntero a funcion para los metodos sobrecargables de un widget. */
typedef widget_t *(*widget_get_focus_fn)(widget_t *self, widget_t *target);
typedef void (*widget_model_to_screen_fn)(widget_t *self, int32_t *x, int32_t *y);
typedef void (*widget_draw_fn)(widget_t *self, graphic_context_t *gc);
typedef void (*widget_on_mouse_down_fn)(widget_t *self, int32_t x, int32_t y, uint8_t button);
typedef void (*widget_on_mouse_up_fn)(widget_t *self, int32_t x, int32_t y, uint8_t button);
typedef void (*widget_on_mouse_move_fn)(widget_t *self, int32_t oldx, int32_t oldy, int32_t newx,
                                        int32_t newy);
typedef void (*widget_on_key_down_fn)(widget_t *self, char c);
typedef void (*widget_on_key_up_fn)(widget_t *self, char c);

/** widget_t - Widget base de la interfaz grafica.
   Cada widget tiene un padre (opcional), coordenadas relativas al padre,
   dimensiones y un color de paleta VGA (indice de 8 bits). */
typedef struct widget {
    struct widget *parent;
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    uint8_t color;
    bool focussable;

    widget_get_focus_fn get_focus;
    widget_model_to_screen_fn model_to_screen;
    widget_draw_fn draw;
    widget_on_mouse_down_fn on_mouse_down;
    widget_on_mouse_up_fn on_mouse_up;
    widget_on_mouse_move_fn on_mouse_move;
    widget_on_key_down_fn on_key_down;
    widget_on_key_up_fn on_key_up;
} widget_t;

/** composite_widget_t - Widget compuesto que contiene widgets hijos.
   Gestiona el dibujo, la despacho de eventos y el foco hacia sus hijos. */
typedef struct {
    widget_t base;
    widget_t *children[WIDGET_CHILDREN_CAPACITY];
    int num_children;
    widget_t *focussed_child;
} composite_widget_t;

/** widget_init - Inicializa un widget base.
   @parent: widget padre (puede ser NULL para la raiz)
   @x,y,w,h: posicion y tamaño relativos al padre
   @color: indice de color de paleta VGA */
void widget_init(widget_t *widget, widget_t *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                 uint8_t color);

/** widget_get_focus - Propaga la peticion de foco hacia la raiz.
   @widget: widget que pide el foco
   @target: widget hijo que recibe el foco
   Return: el widget raiz que gestiona el foco */
widget_t *widget_get_focus(widget_t *widget, widget_t *target);

/** widget_model_to_screen - Convierte coordenadas de modelo a pantalla.
   @widget: widget de referencia
   @x,y: coordenadas (relativas) a convertir, se actualizan a pantalla */
void widget_model_to_screen(widget_t *widget, int32_t *x, int32_t *y);

/** widget_draw - Dibuja el widget sobre un contexto grafico.
   @widget: widget a dibujar
   @gc: contexto grafico de destino */
void widget_draw(widget_t *widget, graphic_context_t *gc);

/** widget_contains - Indica si el punto (en coordenadas relativas al padre
   del widget) cae dentro de su area. compara contra la posicion propia (widget->x/y). */
bool widget_contains(widget_t *widget, int32_t x, int32_t y);

/** composite_widget_init - Inicializa un widget compuesto.
   Iguales parametros que widget_init, agregando la gestion de hijos. */
void composite_widget_init(composite_widget_t *cw, widget_t *parent, int32_t x, int32_t y,
                           int32_t w, int32_t h, uint8_t color);

/** composite_widget_add_child - Agrega un widget hijo al compuesto.
   @cw: widget compuesto
   @child: widget hijo a agregar
   Return: true si se agrego, false si la capacidad esta llena */
bool composite_widget_add_child(composite_widget_t *cw, widget_t *child);

/** composite_widget_get_focus - Establece el hijo enfocado y propaga.
   @cw: widget compuesto
   @widget: hijo que recibe el foco */
widget_t *composite_widget_get_focus(composite_widget_t *cw, widget_t *widget);

/** composite_widget_draw - Dibuja el compuesto y luego sus hijos. */
void composite_widget_draw(composite_widget_t *cw, graphic_context_t *gc);

/** composite_widget_on_mouse_down - Despacha click de un boton a un hijo. */
void composite_widget_on_mouse_down(composite_widget_t *cw, int32_t x, int32_t y, uint8_t button);

/** composite_widget_on_mouse_up - Despacha soltar boton a un hijo. */
void composite_widget_on_mouse_up(composite_widget_t *cw, int32_t x, int32_t y, uint8_t button);

/** composite_widget_on_mouse_move - Despacha movimiento de raton a un hijo. */
void composite_widget_on_mouse_move(composite_widget_t *cw, int32_t oldx, int32_t oldy,
                                    int32_t newx, int32_t newy);

/** composite_widget_on_key_down - Envia tecla al hijo enfocado. */
void composite_widget_on_key_down(composite_widget_t *cw, char c);

/** composite_widget_on_key_up - Envia tecla soltada al hijo enfocado. */
void composite_widget_on_key_up(composite_widget_t *cw, char c);
