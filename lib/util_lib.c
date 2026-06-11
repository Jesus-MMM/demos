/* util_lib.c - Implementacion de funciones de utilidad para el kernel.
   Proporciona operaciones comunes sin depender de la libc estandar. */

#include "util_lib.h"

/** strlen - Calcula la longitud de una cadena terminada en '\0'.
 * @str: puntero a la cadena. Si es NULL retorna -1.
 * Retorna: la cantidad de caracteres antes del terminador nulo,
 *          o -1 si la cadena es NULL. */
int64_t strlen(const char *str)
{
    if (str == NULL) {
        return -1;
    }

    const char *start = str;
    while (*str != '\0') {
        str++;
    }

    return (int64_t)(str - start);
}
