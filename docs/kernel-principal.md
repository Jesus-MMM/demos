# Kernel principal (kernelmain.c)

## Proposito

`kernelmain.c` es el punto de entrada del kernel en C. Sirve como orquestador minimo que delega toda la logica visual a los modulos especializados.

## Contenido completo

```c
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

    style_cursor(DISABLE);
    animate_splash();

    keyboard_set_cursor(0, 0);
    style_cursor(SMALL);
    keyboard_init();

    return 0;
}
```

## Explicacion

### Includes

| Include | Proposito |
|---------|-----------|
| `#include "io.h"` | `style_cursor()` para el cursor, constantes de color |
| `#include "serial.h"` | `serial_init()` para el puerto serie (depuracion) |
| `#include "splash.h"` | `animate_splash()` para la animacion de bienvenida |
| `#include "util_lib.h"` | Funciones de utilidad |
| `#include "gdt.h"` | `init_gdt()` para la Tabla de Descriptores Globales |
| `#include "interrupts.h"` | `init_interrupt_manager()` para la IDT y PIC |
| `#include "keyboard.h"` | `keyboard_init()`, `keyboard_set_cursor()` |

### `style_cursor(DISABLE)`

Desactiva el cursor de texto parpadeante del hardware VGA escribiendo en el puerto CRTC `0x3D4`/`0x3D5`. Esto evita que el cursor distraiga durante la animacion.

### `keyboard_set_cursor(0, 0)` y `style_cursor(SMALL)`

Despues de la animacion, posiciona el cursor en la esquina superior izquierda y lo hace visible en estilo pequeno. Esto prepara la pantalla para recibir entrada del teclado.

### `keyboard_init()`

Inicializa el controlador PS/2: drena el buffer de datos, configura el byte de habilitacion de IRQ1, y envia `0xF4` para activar el escaneo del teclado. A partir de este punto, cualquier tecla presionada genera una IRQ 1 que el kernel atiende.

### Funcion `kernel_main()`

Retorna `int` (aunque nunca se usa, el loader queda en bucle infinito). En un SO real recibiria el magic number de GRUB y un puntero a informacion del hardware.

## Modularidad

El kernel principal se mantiene propositivamente minimalista. Toda la logica compleja vive en modulos separados:

| Modulo | Responsabilidad |
|--------|-----------------|
| `gdt.c` | Tabla de Descriptores Globales (segmentos de memoria) |
| `interrupts.c` | IDT, PIC 8259A, despacho de interrupciones |
| `keyboard.c` | Driver de teclado PS/2 (scancodes → ASCII → pantalla) |
| `serial.c` | Puerto serie UART 16550 (depuracion) |
| `splash.c` | Animacion y layout centrado |
| `big_text.c` | Letras grandes 5x5, dibujo de cajas |
| `timer.c` | Espera activa (delay) |
| `io.c` | Escritura directa en framebuffer VGA |

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Loader en ensamblador](loader-ensamblador.md) | [Librerias del sistema](librerias.md) |

| Relacionados |
|--------------|
| [GDT](gdt.md) |
| [Sistema de interrupciones](interrupts.md) |
| [Driver de teclado](keyboard.md) |
| [Puerto serie](serial.md) |
| [VGA y Framebuffer](vga-framebuffer.md) |
| [Flujo de ejecucion](flujo-ejecucion.md) |
