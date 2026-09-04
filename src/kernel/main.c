/* main.c - Punto de entrada principal del kernel.
   Llamado desde el archivo de arranque en ensamblador (asm/loader.s).
   Inicializa el sistema, registra los drivers y lanza la pantalla de presentacion. */

#include "drivers/driver.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "fs/fstest.h"
#include "fs/vfs.h"
#include "graphic_context.h"
#include "gui/desktop.h"
#include "gui/window.h"
#include "kernel/gdt.h"
#include "kernel/interrupts.h"
#include "kernel/pci.h"
#include "types.h"
#include "util/png.h"
#include "util/splash.h"
#include "util/util_lib.h"

/* Drivers estaticos - se registran en el administrador */
static keyboard_driver_t kb_driver;
static mouse_driver_t ms_driver;

static graphic_context_t gc;
static desktop_t desktop;
static window_t win1;
static window_t win2;

/* Pixels del fondo del escritorio (320x200, indices de la paleta VGA por
   defecto). Se compone al back buffer en cada frame por desktop_draw, por lo
   que no se sobrescribe. */
static uint8_t background_pixels[320 * 200];
static uint8_t vga_palette[256][3];

/* Lee /FONDO.PNG del disco (nombre FAT 8.3), lo decodifica con la herramienta
   PNG del kernel y lo deja listo como fondo persistente
   del escritorio. La conversion a indices usa la paleta VGA activa (la que el
   modo 320x200x8 tiene por defecto). */
static void background_load_from_disk(void)
{
    static uint8_t file_data[PNG_FILE_MAX];
    int fd = vfs_open("/FONDO.PNG", 1);
    if (fd < 0) {
        return;
    }

    uint32_t total = 0;
    for (;;) {
        int n = vfs_read(fd, file_data + total, (uint32_t)(sizeof(file_data) - total));
        if (n <= 0) {
            break;
        }
        total += (uint32_t)n;
    }
    vfs_close(fd);
    if (total < 8) {
        return;
    }

    vga_read_palette(vga_palette);

    uint32_t w = 0;
    uint32_t h = 0;
    bool r = png_decode_indexed(file_data, total, vga_palette, background_pixels,
                               (uint32_t)sizeof(background_pixels), &w, &h);
    if (r != false) {
        return;
    }

    desktop_set_background(&desktop, background_pixels, w, h);
}

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

    /* Probar el filesystem FAT32 montando el disco ATA y validarlo */
    fs_test_run();

    vga_set_mode(320, 200, 8);
    vga_fill_rectangle(0, 0, 320, 200, 0x09);
    background_load_from_disk();

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
