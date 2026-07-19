/* util_lib.c - Implementacion de funciones de utilidad para el kernel.
   Proporciona operaciones comunes sin depender de la libc estandar. */

#include "util/util_lib.h"

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
