/* asm.h - Operaciones de E/S sobre puertos usando ensamblador inline.
   Proporciona acceso a los puertos de E/S del procesador x86. */

#pragma once

#include "types.h"

/** outb - Escribe un byte en un puerto de E/S.
 * @port:  direccion del puerto (16 bits)
 * @value: byte a escribir
 * Nota: los parametros son intercambiables por tipo, pero el inline
 * assembly usa la convencion "a"(value), "Nd"(port). */
static inline void outb(uint16_t port, // NOLINT(bugprone-easily-swappable-parameters)
                        uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

/** inb - Lee un byte desde un puerto de E/S.
 * @port: direccion del puerto (16 bits)
 * Retorna: el byte leido. */
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
