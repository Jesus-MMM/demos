#include "gui/widget.h"

void widget_on_mouse_down_default(widget_t *self, int32_t x, int32_t y, uint8_t button)
{
    (void)x;
    (void)y;
    (void)button;
    if (self->focussable) {
        widget_get_focus(self, self);
    }
}

void widget_on_mouse_up_default(widget_t *self, int32_t x, int32_t y, uint8_t button)
{
    (void)self;
    (void)x;
    (void)y;
    (void)button;
}

void widget_on_mouse_move_default(widget_t *self, int32_t oldx, int32_t oldy, int32_t newx,
                                  int32_t newy)
{
    (void)self;
    (void)oldx;
    (void)oldy;
    (void)newx;
    (void)newy;
}

void widget_on_key_down_default(widget_t *self, char c)
{
    (void)self;
    (void)c;
}

void widget_on_key_up_default(widget_t *self, char c)
{
    (void)self;
    (void)c;
}

void widget_init(widget_t *widget, widget_t *parent, int32_t x, int32_t y, int32_t w, int32_t h,
                 uint8_t color) // NOLINT(bugprone-easily-swappable-parameters)
{
    widget->parent = parent;
    widget->x = x;
    widget->y = y;
    widget->w = w;
    widget->h = h;
    widget->color = color;
    widget->focussable = true;

    widget->get_focus = (widget_get_focus_fn)widget_get_focus;
    widget->model_to_screen = (widget_model_to_screen_fn)widget_model_to_screen;
    widget->draw = (widget_draw_fn)widget_draw;
    widget->on_mouse_down = (widget_on_mouse_down_fn)widget_on_mouse_down_default;
    widget->on_mouse_up = (widget_on_mouse_up_fn)widget_on_mouse_up_default;
    widget->on_mouse_move = (widget_on_mouse_move_fn)widget_on_mouse_move_default;
    widget->on_key_down = (widget_on_key_down_fn)widget_on_key_down_default;
    widget->on_key_up = (widget_on_key_up_fn)widget_on_key_up_default;
}

widget_t *widget_get_focus(widget_t *widget, widget_t *target)
{
    if (widget->parent != NULL) {
        return widget->parent->get_focus(widget->parent, target);
    }
    return NULL;
}

void widget_model_to_screen(widget_t *widget, int32_t *x, int32_t *y)
{
    if (widget->parent != NULL) {
        widget->parent->model_to_screen(widget->parent, x, y);
    }
    *x += widget->x;
    *y += widget->y;
}

void widget_draw(widget_t *widget, graphic_context_t *gc)
{
    int32_t x = 0;
    int32_t y = 0;
    widget_model_to_screen(widget, &x, &y);
    graphic_context_fill_rectangle(gc, x, y, widget->w, widget->h, widget->color);
}

bool widget_contains(widget_t *widget, int32_t x, int32_t y)
{
    return widget->x <= x && x < widget->x + widget->w && widget->y <= y &&
           y < widget->y + widget->h;
}

/* ---------------------------------------------------------------- */
/* Widget compuesto                                                 */
/* ---------------------------------------------------------------- */

void composite_widget_init(composite_widget_t *cw, widget_t *parent, int32_t x, int32_t y,
                           int32_t w, int32_t h, uint8_t color)
{
    widget_init(&cw->base, parent, x, y, w, h, color);
    cw->num_children = 0;
    cw->focussed_child = NULL;

    cw->base.get_focus = (widget_get_focus_fn)composite_widget_get_focus;
    cw->base.model_to_screen = (widget_model_to_screen_fn)widget_model_to_screen;
    cw->base.draw = (widget_draw_fn)composite_widget_draw;
    cw->base.on_mouse_down = (widget_on_mouse_down_fn)composite_widget_on_mouse_down;
    cw->base.on_mouse_up = (widget_on_mouse_up_fn)composite_widget_on_mouse_up;
    cw->base.on_mouse_move = (widget_on_mouse_move_fn)composite_widget_on_mouse_move;
    cw->base.on_key_down = (widget_on_key_down_fn)composite_widget_on_key_down;
    cw->base.on_key_up = (widget_on_key_up_fn)composite_widget_on_key_up;
}

bool composite_widget_add_child(composite_widget_t *cw, widget_t *child)
{
    if (cw->num_children >= WIDGET_CHILDREN_CAPACITY) {
        return false;
    }
    cw->children[cw->num_children] = child;
    cw->num_children++;
    child->parent = &cw->base;
    return true;
}

widget_t *composite_widget_get_focus(composite_widget_t *cw, widget_t *widget)
{
    cw->focussed_child = widget;
    if (cw->base.parent != NULL) {
        return cw->base.parent->get_focus(cw->base.parent, &cw->base);
    }
    return NULL;
}

void composite_widget_draw(composite_widget_t *cw, graphic_context_t *gc)
{
    widget_draw(&cw->base, gc);
    for (int i = cw->num_children - 1; i >= 0; --i) {
        widget_draw(cw->children[i], gc);
    }
}

void composite_widget_on_mouse_down(composite_widget_t *cw, int32_t x, int32_t y, uint8_t button)
{
    for (int i = 0; i < cw->num_children; ++i) {
        if (widget_contains(cw->children[i], x - cw->base.x, y - cw->base.y)) {
            if (cw->children[i]->on_mouse_down != NULL) {
                cw->children[i]->on_mouse_down(cw->children[i], x - cw->base.x, y - cw->base.y,
                                               button);
            }
            break;
        }
    }
}

void composite_widget_on_mouse_up(composite_widget_t *cw, int32_t x, int32_t y, uint8_t button)
{
    for (int i = 0; i < cw->num_children; ++i) {
        if (widget_contains(cw->children[i], x - cw->base.x, y - cw->base.y)) {
            if (cw->children[i]->on_mouse_up != NULL) {
                cw->children[i]->on_mouse_up(cw->children[i], x - cw->base.x, y - cw->base.y,
                                             button);
            }
            break;
        }
    }
}

void composite_widget_on_mouse_move(composite_widget_t *cw, int32_t oldx, int32_t oldy,
                                    int32_t newx, int32_t newy)
{
    int firstchild = -1;
    for (int i = 0; i < cw->num_children; ++i) {
        if (widget_contains(cw->children[i], oldx - cw->base.x, oldy - cw->base.y)) {
            if (cw->children[i]->on_mouse_move != NULL) {
                cw->children[i]->on_mouse_move(cw->children[i], oldx - cw->base.x,
                                               oldy - cw->base.y, newx - cw->base.x,
                                               newy - cw->base.y);
            }
            firstchild = i;
            break;
        }
    }

    for (int i = 0; i < cw->num_children; ++i) {
        if (widget_contains(cw->children[i], newx - cw->base.x, newy - cw->base.y)) {
            if (firstchild != i && cw->children[i]->on_mouse_move != NULL) {
                cw->children[i]->on_mouse_move(cw->children[i], oldx - cw->base.x,
                                               oldy - cw->base.y, newx - cw->base.x,
                                               newy - cw->base.y);
            }
            break;
        }
    }
}

void composite_widget_on_key_down(composite_widget_t *cw, char c)
{
    if (cw->focussed_child != NULL && cw->focussed_child->on_key_down != NULL) {
        cw->focussed_child->on_key_down(cw->focussed_child, c);
    }
}

void composite_widget_on_key_up(composite_widget_t *cw, char c)
{
    if (cw->focussed_child != NULL && cw->focussed_child->on_key_up != NULL) {
        cw->focussed_child->on_key_up(cw->focussed_child, c);
    }
}
