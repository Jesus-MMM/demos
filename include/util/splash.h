/* splash.h - Pantalla de presentacion animada del sistema operativo. */

#pragma once

#include "drivers/timer.h"
#include "drivers/vga_legacy.h"
#include "util/big_text.h"

/** animate_splash - Muestra una animacion de inicio del sistema operativo
 * en pantalla, con graficos y efectos visuales. Se ejecuta despues de la
 * inicializacion basica del kernel. */
void animate_splash(void);
