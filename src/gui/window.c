#include "gui/window.h"

#define WINDOW_TITLE_BAR_HEIGHT 15

void window_init(window_t *win, widget_t *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                 uint8_t color, char *label)
{
    composite_widget_init(&win->base, parent, x, y, w, h, color);
    win->label = label;
    win->dragging = false;

    win->base.base.get_focus = (widget_get_focus_fn)widget_get_focus;
    win->base.base.model_to_screen = (widget_model_to_screen_fn)widget_model_to_screen;
    win->base.base.draw = (widget_draw_fn)window_draw;
    win->base.base.on_mouse_down = (widget_on_mouse_down_fn)window_on_mouse_down;
    win->base.base.on_mouse_up = (widget_on_mouse_up_fn)window_on_mouse_up;
    win->base.base.on_mouse_move = (widget_on_mouse_move_fn)window_on_mouse_move;
}

void window_draw(window_t *win, graphic_context_t *gc)
{
    composite_widget_draw(&win->base, gc);

    if (win->label == NULL) {
        return;
    }

    /* Barra de titulo: se tapa la parte superior de la ventana con un color
       de borde y se deja la etiqueta reservada para renderizado posterior. */
    int32_t x = 0;
    int32_t y = 0;
    widget_model_to_screen(&win->base.base, &x, &y);

    graphic_context_fill_rectangle(gc, x, y, win->base.base.w, WINDOW_TITLE_BAR_HEIGHT, 0x08);
}

void window_on_mouse_down(window_t *win, int32_t x, int32_t y,
                          uint8_t button) // NOLINT(bugprone-easily-swappable-parameters)
{
    widget_get_focus(&win->base.base, &win->base.base);
    win->dragging = (button == 0);
    composite_widget_on_mouse_down(&win->base, x, y, button);
}

void window_on_mouse_up(window_t *win, int32_t x, int32_t y,
                        uint8_t button) // NOLINT(bugprone-easily-swappable-parameters)
{
    win->dragging = false;
    composite_widget_on_mouse_up(&win->base, x, y, button);
}

void window_on_mouse_move(window_t *win, int32_t oldx, int32_t oldy,
                          int32_t newx, // NOLINT(bugprone-easily-swappable-parameters)
                          int32_t newy)
{
    if (win->dragging) {
        win->base.base.x += newx - oldx;
        win->base.base.y += newy - oldy;
    }
    composite_widget_on_mouse_move(&win->base, oldx, oldy, newx, newy);
}
