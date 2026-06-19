/* kernelmain.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (loader.s).
   Inicializa el sistema y lanza la pantalla de presentacion. */

#include "io.h"
#include "serial.h"
#include "splash.h"
#include "util_lib.h"

/** kernel_main - Punto de entrada principal.
 * Inicializa el puerto serie para depuracion, desactiva el cursor de
 * hardware y reproduce la animacion de presentacion.
 *
 * Return: 0 si la inicializacion fue exitosa. */
int kernel_main()
{

    serial_init(COM1_BASE_ADDRESS);

    const char *test = "Inicializando DEMOS";

    serial_write_string(COM1_BASE_ADDRESS, test, strlen(test));

    style_cursor(DISABLE);
    animate_splash();
    return 0;
}
