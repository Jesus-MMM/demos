/* util_lib.h - Funciones de utilidad varias para el kernel. */

#pragma once

#include "types.h"

/** strlen - Calcula la longitud de una cadena terminada en nulo.
 * @str: puntero a la cadena
 * Retorna: la longitud en caracteres, o -1 si @str es NULL. */
int64_t strlen(const char *str);
