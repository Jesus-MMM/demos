#include "gui/desktop.h"

#define DESKTOP_WIDTH 320
#define DESKTOP_HEIGHT 200

void desktop_init(desktop_t *desktop, graphic_context_t *gc)
{
    composite_widget_init(&desktop->base, NULL, 0, 0, DESKTOP_WIDTH, DESKTOP_HEIGHT, 0x09);

    desktop->base.base.get_focus = (widget_get_focus_fn)widget_get_focus;
    desktop->base.base.model_to_screen = (widget_model_to_screen_fn)widget_model_to_screen;
    desktop->base.base.draw = (widget_draw_fn)desktop_draw;
    desktop->base.base.on_mouse_down = (widget_on_mouse_down_fn)composite_widget_on_mouse_down;
    desktop->base.base.on_mouse_up = (widget_on_mouse_up_fn)composite_widget_on_mouse_up;
    desktop->base.base.on_mouse_move = (widget_on_mouse_move_fn)composite_widget_on_mouse_move;
    desktop->base.base.on_key_down = (widget_on_key_down_fn)composite_widget_on_key_down;
    desktop->base.base.on_key_up = (widget_on_key_up_fn)composite_widget_on_key_up;

    desktop->mouse_x = DESKTOP_WIDTH / 2;
    desktop->mouse_y = DESKTOP_HEIGHT / 2;
    desktop->width = DESKTOP_WIDTH;
    desktop->height = DESKTOP_HEIGHT;

    (void)gc;
}

void desktop_draw(desktop_t *desktop, graphic_context_t *gc)
{
    composite_widget_draw(&desktop->base, gc);

    for (int32_t i = 0; i < 4; i++) {
        graphic_context_put_pixel(gc, desktop->mouse_x - i, desktop->mouse_y, 0x0F);
        graphic_context_put_pixel(gc, desktop->mouse_x + i, desktop->mouse_y, 0x0F);
        graphic_context_put_pixel(gc, desktop->mouse_x, desktop->mouse_y - i, 0x0F);
        graphic_context_put_pixel(gc, desktop->mouse_x, desktop->mouse_y + i, 0x0F);
    }
}

void desktop_on_mouse_move(int8_t x_offset, int8_t y_offset, void *data)
{
    desktop_t *desktop = (desktop_t *)data;

    x_offset /= 4;
    y_offset /= 4;

    int32_t new_mouse_x = desktop->mouse_x + x_offset;
    if (new_mouse_x < 0) {
        new_mouse_x = 0;
    }
    if (new_mouse_x >= desktop->width) {
        new_mouse_x = desktop->width - 1;
    }

    int32_t new_mouse_y = desktop->mouse_y - y_offset;
    if (new_mouse_y < 0) {
        new_mouse_y = 0;
    }
    if (new_mouse_y >= desktop->height) {
        new_mouse_y = desktop->height - 1;
    }

    composite_widget_on_mouse_move(&desktop->base, desktop->mouse_x, desktop->mouse_y, new_mouse_x,
                                   new_mouse_y);

    desktop->mouse_x = new_mouse_x;
    desktop->mouse_y = new_mouse_y;
}

void desktop_on_mouse_button(uint8_t button, int8_t x, int8_t y, bool pressed, void *data)
{
    desktop_t *desktop = (desktop_t *)data;
    (void)x;
    (void)y;

    if (button != 0) {
        return;
    }

    if (pressed) {
        composite_widget_on_mouse_down(&desktop->base, desktop->mouse_x, desktop->mouse_y, button);
    } else {
        composite_widget_on_mouse_up(&desktop->base, desktop->mouse_x, desktop->mouse_y, button);
    }
}

void desktop_on_key_down(char c, void *data)
{
    desktop_t *desktop = (desktop_t *)data;
    composite_widget_on_key_down(&desktop->base, c);
}

void desktop_on_key_up(char c, void *data)
{
    desktop_t *desktop = (desktop_t *)data;
    composite_widget_on_key_up(&desktop->base, c);
}
