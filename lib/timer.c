/* timer.c - Implementacion de temporizacion por espera activa (busy-wait).
   Utiliza un bucle vacio con volatile para evitar que el optimizador
   elimine la espera. Util cuando no hay un PIT/APIC configurado. */

#include "timer.h"

void delay(uint32_t iterations)
{
    for (volatile uint32_t i = 0; i < iterations; i++) {
        ;
    }
}
