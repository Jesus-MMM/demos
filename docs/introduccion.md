# Introduccion a DemOS

**DemOS** es un sistema operativo minimalista desarrollado desde cero con fines educativos.

## Que hace DemOS?

Al iniciarse, DemOS:

1. Carga el kernel mediante **GRUB** usando la especificacion **Multiboot**.
2. Inicializa la pila (stack) del kernel.
3. Configura la **GDT** (Tabla de Descriptores Globales) con segmentos de codigo y datos.
4. Inicializa la **IDT** (Tabla de Descriptores de Interrupcion) y el **PIC 8259A**.
5. Inicializa el **driver del teclado PS/2** (IRQ 1).
6. Inicializa el **driver del mouse PS/2** (IRQ 12).
7. Habilita las interrupciones del CPU (`sti`).
8. Habilita el cursor y queda listo para recibir entrada.

## Estructura del proyecto

```
DemOS/
├── asm/
│   ├── loader.s              # Arranque (Multiboot + stack)
│   └── interruptstubs.s      # Stubs en ensamblador para interrupciones
├── src/
│   ├── kernel/
│   │   ├── main.c            # Entry point del kernel
│   │   ├── gdt.c             # GDT, descriptores de segmento
│   │   └── interrupts.c      # IDT, PIC 8259A, despacho de interrupciones
│   ├── drivers/
│   │   ├── keyboard.c        # Driver de teclado PS/2
│   │   ├── mouse.c           # Driver de mouse PS/2
│   │   ├── serial.c          # Puerto serie (UART 16550)
│   │   ├── timer.c           # delay() busy-wait
│   │   └── vga.c             # Framebuffer, cursor, scroll
│   └── util/
│       ├── big_text.c        # Glyphs, draw_box, draw_big_char
│       ├── splash.c          # Animacion de bienvenida
│       └── util_lib.c        # strlen
├── include/
│   ├── types.h               # Tipos enteros exactos
│   ├── asm.h                 # Instrucciones in/out (inline asm)
│   ├── kernel/
│   │   ├── gdt.h             # Tabla de Descriptores Globales
│   │   └── interrupts.h      # IDT y manejadores de interrupcion
│   ├── drivers/
│   │   ├── keyboard.h        # Driver de teclado PS/2
│   │   ├── mouse.h           # Driver de mouse PS/2
│   │   ├── serial.h          # Puerto serie (UART 16550)
│   │   ├── timer.h           # delay()
│   │   └── vga.h             # E/S pantalla VGA
│   └── util/
│       ├── big_text.h        # Glyphs, draw_box, draw_big_char
│       ├── splash.h          # animate_splash()
│       └── util_lib.h        # strlen
├── link.ld                   # Script de enlace
├── Makefile                  # Compilacion
├── grub.cfg                  # Configuracion GRUB
├── build/                    # Objetos compilados
├── iso/                      # Imagen ISO
└── DemOS.iso                 # ISO booteable final
```

## Navegacion

| Documento | Descripcion |
|-----------|-------------|
| [Arquitectura general](arquitectura.md) | Vision general del sistema |
| [Entorno y herramientas](entorno-setup.md) | Requisitos para compilar y ejecutar |
| [Sistema de compilacion](makefile.md) | Explicacion del Makefile |
| [Loader en ensamblador](loader-ensamblador.md) | El punto de entrada del kernel |
| [Kernel principal](kernel-principal.md) | Corazon del sistema operativo |
| [Librerias del sistema](librerias.md) | Tipos, E/S, timer, big_text, splash |
| [Puerto serie](serial.md) | UART 16550 para depuracion |
| [GDT](gdt.md) | Tabla de Descriptores Globales |
| [Sistema de interrupciones](interrupts.md) | IDT, PIC 8259A, interrupt stubs |
| [Driver de teclado](keyboard.md) | PS/2 keyboard driver |
| [Driver de mouse](mouse.md) | PS/2 mouse driver |
| [VGA y Framebuffer](vga-framebuffer.md) | Como se escribe en pantalla |
| [Flujo de ejecucion](flujo-ejecucion.md) | Recorrido completo del codigo |
