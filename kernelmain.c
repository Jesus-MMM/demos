/* kernelmain.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (loader.s).
   Inicializa el sistema y lanza la pantalla de presentacion. */

#include "io.h"
#include "serial.h"
#include "splash.h"
#include "util_lib.h"
#include "gdt.h"

int kernel_main()
{

    serial_init(COM1_BASE_ADDRESS);

    const char *test = "Inicializando DEMOS";

    global_descriptor_table gdt;

    init_gdt(&gdt);

    serial_write_string(COM1_BASE_ADDRESS, test, strlen(test));

    style_cursor(DISABLE);
    animate_splash();
    return 0;
}
