/* kernelmain.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (loader.s).
   Inicializa el sistema y lanza la pantalla de presentacion. */

#include "io.h"
#include "splash.h"

int kernel_main()
{
    style_cursor(DISABLE);
    animate_splash();
    return 0;
}
