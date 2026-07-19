/* keyboard.h - Driver del teclado PS/2.
   Maneja interrupciones IRQ 1 (0x21), traduce scancodes a ASCII
   y escribe los caracteres en la pantalla VGA. */

#pragma once

#include "types.h"

/** keyboard_init - Inicializa el controlador PS/2 y habilita el teclado.
   Drena el buffer de datos, activa las interrupciones del controlador,
   configura el byte de comando y envia 0xF4 para activar el escaneo. */
void keyboard_init(void);

/** keyboard_handler - Manejador de interrupcion del teclado (IRQ 1).
   Lee el scancode del puerto de datos 0x60, lo traduce a ASCII
   usando la tabla de scancodes y escribe el caracter en pantalla.
   @esp: puntero a la pila al momento de la interrupcion
   Retorna: el puntero de pila sin modificar */
uint32_t keyboard_handler(uint32_t esp);

/** keyboard_set_cursor - Establece la posicion del cursor de escritura.
   @row: fila (0-24)
   @col: columna (0-79) */
void keyboard_set_cursor(uint16_t row, uint16_t col);
