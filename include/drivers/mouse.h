/* mouse.h - Driver del raton PS/2.
   Maneja interrupciones IRQ 12 (0x2C), decodifica paquetes de 3 bytes
   del raton y notifica movimientos y clics a traves de callbacks. */

#pragma once

#include "asm.h"
#include "drivers/driver.h"
#include "drivers/serial.h"
#include "drivers/vga_legacy.h"
#include "types.h"

#define MS_DATA_PORT 0x60
#define MS_COMMAND_PORT 0x64

/** mouse_driver_t - Estructura del driver de raton.
   Contiene el driver base y estado especifico del raton:
   buffer de 3 bytes, posicion, boton y callbacks de eventos. */
typedef struct {
    driver_t base;

    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;
    int8_t x;
    int8_t y;

    void *handler_data;
    on_mouse_move_fn on_mouse_move;
    on_mouse_button_fn on_mouse_button;
} mouse_driver_t;

/** mouse_driver_init - Inicializa un mouse_driver_t y su driver base.
   @drv:     puntero al driver de raton
   @on_move: callback para movimientos (NULL = handler VGA default)
   @data:    dato asociado al handler (puede ser NULL) */
void mouse_driver_init(mouse_driver_t *drv, on_mouse_move_fn on_move, void *data);

/** mouse_default_on_move - Handler por defecto que dibuja un cursor VGA.
   @x_offset: desplazamiento horizontal
   @y_offset: desplazamiento vertical
   @data:     puntero al mouse_driver_t (cast interno) */
void mouse_default_on_move(int8_t x_offset, int8_t y_offset, void *data);
