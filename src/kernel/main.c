/* main.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (asm/loader.s).
   Inicializa el sistema, registra los drivers y lanza la pantalla de presentacion. */

#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "kernel/gdt.h"
#include "kernel/interrupts.h"
#include "kernel/pci.h"
#include "util/splash.h"
#include "util/util_lib.h"

/* Drivers estaticos - se registran en el administrador */
static keyboard_driver_t kb_driver;
static mouse_driver_t ms_driver;

int kernel_main()
{
    static global_descriptor_table gdt;

    serial_init(COM1_BASE_ADDRESS);
    init_gdt(&gdt);
    init_interrupt_manager(&gdt);

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 1\n", 39);

    /* Inicializar el administrador de drivers */
    driver_manager_init(&global_driver_manager);

    /* Crear el driver de teclado con handler VGA por defecto */
    keyboard_driver_init(&kb_driver, keyboard_default_on_key_down, &kb_driver);
    driver_manager_add(&global_driver_manager, &kb_driver.base);

    /* Crear el driver de raton con handler VGA por defecto */
    mouse_driver_init(&ms_driver, mouse_default_on_move, &ms_driver);
    driver_manager_add(&global_driver_manager, &ms_driver.base);

    select_drivers(&global_driver_manager);

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 2\n", 39);

    /* Activar todos los drivers registrados (PS/2 keyboard + mouse) */
    driver_manager_activate_all(&global_driver_manager);

    /* Posicionar cursor y estilo */
    keyboard_set_cursor(0, 0);
    style_cursor(SMALL);

    vga_set_mode(320, 200, 8);

    for (uint32_t y = 0; y < 200; y++) {
        for (uint32_t x = 0; x < 320; x++) {
            vga_write_pixel(x, y, 0x4);
        }
    }

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 3\n", 39);

    /* Habilitar interrupciones - los drivers ya estan listos */
    asm volatile("sti");

    return 0;
}
