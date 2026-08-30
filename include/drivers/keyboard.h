/* keyboard.h - Driver del teclado PS/2.
   Maneja interrupciones IRQ 1 (0x21), traduce scancodes a ASCII
   y notifica a traves de callbacks de eventos (on_key_down/on_key_up).
   Incluye un handler por defecto que escribe en pantalla VGA. */

#pragma once

#include "asm.h"
#include "drivers/driver.h"
#include "drivers/serial.h"
#include "drivers/vga_legacy.h"
#include "types.h"

#define KB_DATA_PORT 0x60
#define KB_COMMAND_PORT 0x64
#define SCREEN_COLS 80
#define SCREEN_ROWS 25

/** keyboard_driver_t - Estructura del driver de teclado.
   Contiene el driver base y estado especifico del teclado:
   posicion del cursor, puntero a datos del handler, y callbacks de eventos. */
typedef struct {
    driver_t base;

    uint16_t cursor_row;
    uint16_t cursor_col;

    void *handler_data;
    on_key_down_fn on_key_down;
    on_key_up_fn on_key_up;
} keyboard_driver_t;

/** keyboard_driver_init - Inicializa un keyboard_driver_t y su driver base.
   @drv:            puntero al driver de teclado
   @on_key_down_fn: callback para teclas presionadas (NULL = handler VGA default)
   @handler_data:   dato asociado al handler (puede ser NULL) */
void keyboard_driver_init(keyboard_driver_t *drv, on_key_down_fn on_key_down_fn,
                          void *handler_data);

/** keyboard_default_on_key_down - Handler por defecto que imprime el caracter
   en la pantalla VGA en la posicion del cursor del teclado.
   @c:    caracter ASCII a imprimir
   @data: puntero al keyboard_driver_t (cast interno) */
void keyboard_default_on_key_down(char c, void *data);

/** keyboard_set_cursor - Establece la posicion del cursor de escritura.
   @row: fila (0-24)
   @col: columna (0-79) */
void keyboard_set_cursor(uint16_t row, uint16_t col);
