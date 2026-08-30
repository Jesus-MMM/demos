/* main.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (asm/loader.s).
   Inicializa el sistema, registra los drivers y lanza la pantalla de presentacion. */

#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "graphic_context.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "kernel/gdt.h"
#include "kernel/interrupts.h"
#include "kernel/pci.h"
#include "util/splash.h"
#include "util/util_lib.h"

/* Drivers estaticos - se registran en el administrador */
static keyboard_driver_t kb_driver;
static mouse_driver_t ms_driver;

static graphic_context_t gc;
static desktop_t desktop;
static window_t win1;
static window_t win2;

int kernel_main()
{
    static global_descriptor_table gdt;

    serial_init(COM1_BASE_ADDRESS);
    init_gdt(&gdt);
    init_interrupt_manager(&gdt);

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 1\n", 39);

    /* Inicializar el administrador de drivers */
    driver_manager_init(&global_driver_manager);

    /* Modo grafico: el escritorio recibe los eventos de teclado y raton */
    graphic_context_init(&gc);
    desktop_init(&desktop, &gc);

    keyboard_driver_init(&kb_driver, desktop_on_key_down, &desktop);
    kb_driver.on_key_up = desktop_on_key_up;
    driver_manager_add(&global_driver_manager, &kb_driver.base);

    mouse_driver_init(&ms_driver, desktop_on_mouse_move, &desktop);
    ms_driver.on_mouse_button = desktop_on_mouse_button;
    driver_manager_add(&global_driver_manager, &ms_driver.base);

    select_drivers(&global_driver_manager);

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 2\n", 39);

    /* Activar todos los drivers registrados (PS/2 keyboard + mouse) */
    driver_manager_activate_all(&global_driver_manager);

    vga_set_mode(320, 200, 8);
    vga_fill_rectangle(0, 0, 320, 200, 0x09);

    /* Ventanas de ejemplo sobre el escritorio */
    window_init(&win1, &desktop.base.base, 10, 10, 20, 20, 0x04, NULL);
    composite_widget_add_child(&desktop.base, &win1.base.base);
    window_init(&win2, &desktop.base.base, 40, 15, 30, 30, 0x02, NULL);
    composite_widget_add_child(&desktop.base, &win2.base.base);

    serial_write_string(COM1_BASE_ADDRESS, "[KERNEL] Initializing hardware, Stage 3\n", 39);

    /* Habilitar interrupciones - los drivers ya estan listos */
    asm volatile("sti");

    for (;;) {
        desktop_draw(&desktop, &gc);
        graphic_context_flush(&gc);
    }

    return 0;
}
