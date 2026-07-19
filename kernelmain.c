/* kernelmain.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (loader.s).
   Inicializa el sistema y lanza la pantalla de presentacion. */

#include "io.h"
#include "serial.h"
#include "splash.h"
#include "util_lib.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"

int kernel_main()
{
    static global_descriptor_table gdt;

    serial_init(COM1_BASE_ADDRESS);
    init_gdt(&gdt);
    init_interrupt_manager(&gdt);

    // style_cursor(DISABLE);
    // animate_splash();

    keyboard_set_cursor(0, 0);
    style_cursor(SMALL);
    keyboard_init();

    return 0;
}
