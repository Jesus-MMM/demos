---
title: "Kernel principal"
order: 3
---

# Kernel principal (src/kernel/main.c)

## Proposito

`src/kernel/main.c` es el punto de entrada del kernel en C. Sirve como orquestador minimo que delega toda la logica visual a los modulos especializados.

## Contenido completo

```c
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
```

## Explicacion

### Includes

| Include | Proposito |
|---------|-----------|
| `#include "drivers/vga.h"` | `style_cursor()` para el cursor, constantes de color |
| `#include "drivers/serial.h"` | `serial_init()` para el puerto serie (depuracion) |
| `#include "util/splash.h"` | `animate_splash()` para la animacion de bienvenida |
| `#include "util/util_lib.h"` | Funciones de utilidad |
| `#include "kernel/gdt.h"` | `init_gdt()` para la Tabla de Descriptores Globales |
| `#include "kernel/interrupts.h"` | `init_interrupt_manager()` para la IDT y PIC |
| `#include "kernel/pci.h"` | `select_drivers()` para la enumeracion del bus PCI |
| `#include "drivers/driver.h"` | `driver_manager_t`, `driver_t` y `driver_manager_add()` |
| `#include "drivers/keyboard.h"` | `keyboard_init()`, `keyboard_set_cursor()` |

### `style_cursor(DISABLE)`

Desactiva el cursor de texto parpadeante del hardware VGA escribiendo en el puerto CRTC `0x3D4`/`0x3D5`. Esto evita que el cursor distraiga durante la animacion.

### `keyboard_set_cursor(0, 0)` y `style_cursor(SMALL)`

Despues de la animacion, posiciona el cursor en la esquina superior izquierda y lo hace visible en estilo pequeno. Esto prepara la pantalla para recibir entrada del teclado.

### `keyboard_init()`

Inicializa el controlador PS/2 del teclado: drena el buffer de datos, configura el byte de habilitacion de IRQ1 con guards IBF/OBF, y envia `0xF4` para activar el escaneo.

### `mouse_init()`

Inicializa el controlador PS/2 del mouse: habilita el segundo puerto PS/2, configura IRQ12 en el byte de control, y envia `0xF4` via el prefijo `0xD4` para activar el envio de paquetes.

### `asm volatile("sti")`

Habilita las interrupciones del CPU. Se llama despues de inicializar teclado y mouse para evitar que interrupciones lleguen antes de que los handlers esten listos.

### Funcion `kernel_main()`

Retorna `int` (aunque nunca se usa, el loader queda en bucle infinito). En un SO real recibiria el magic number de GRUB y un puntero a informacion del hardware.

## Modularidad

El kernel principal se mantiene propositivamente minimalista. Toda la logica compleja vive en modulos separados:

| Modulo | Responsabilidad |
|--------|-----------------|
| `src/kernel/gdt.c` | Tabla de Descriptores Globales (segmentos de memoria) |
| `src/kernel/interrupts.c` | IDT, PIC 8259A, despacho de interrupciones |
| `src/kernel/pci.c` | Enumeracion PCI, deteccion de dispositivos y drivers |
| `src/drivers/keyboard.c` | Driver de teclado PS/2 (scancodes → ASCII → pantalla) |
| `src/drivers/mouse.c` | Driver de mouse PS/2 (IRQ 12, movimiento y botones) |
| `src/drivers/serial.c` | Puerto serie UART 16550 (depuracion) |
| `src/util/splash.c` | Animacion y layout centrado |
| `src/util/big_text.c` | Letras grandes 5x5, dibujo de cajas |
| `src/drivers/timer.c` | Espera activa (delay) |
| `src/drivers/vga.c` | Escritura directa en framebuffer VGA |
