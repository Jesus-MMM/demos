/* timer.h - Funciones de temporizacion por bucle de espera activa (busy-wait). */

#pragma once

#include "types.h"

/** delay - Espera activa durante un numero aproximado de iteraciones.
 * @iterations: cantidad de iteraciones del bucle vacio.
 * Nota: la duracion depende de la velocidad del CPU. */
void delay(uint32_t iterations);
