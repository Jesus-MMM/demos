/* main.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (asm/loader.s).
   Inicializa el sistema y lanza la pantalla de presentacion. */

#include "drivers/vga.h"
#include "drivers/serial.h"
#include "util/splash.h"
#include "util/util_lib.h"
#include "kernel/gdt.h"
#include "kernel/interrupts.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"

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
    mouse_init();
    asm volatile("sti");

    return 0;
}
