# Introduccion a DemOS

**DemOS** es un sistema operativo minimalista desarrollado desde cero con fines educativos.

## Que hace DemOS?

Al iniciarse, DemOS:

1. Carga el kernel mediante **GRUB** usando la especificacion **Multiboot**.
2. Inicializa la pila (stack) del kernel.
3. Configura la **GDT** (Tabla de Descriptores Globales) con segmentos de codigo y datos.
4. Inicializa la **IDT** (Tabla de Descriptores de Interrupcion) y el **PIC 8259A**.
5. Inicializa el **driver del teclado PS/2** (IRQ 1).
6. Desactiva el cursor parpadeante.
7. Dibuja una **caja** centrada en la pantalla con bordes dobles.
8. Anima el texto **"DemOS"** en letras grandes (5x5) dentro de la caja, con transiciones de color verde.
9. Habilita el cursor y queda listo para recibir teclado.

## Estructura del proyecto

```
DemOS/
├── loader.s                 # Arranque (Multiboot + stack)
├── interruptstubs.s         # Stubs en ensamblador para interrupciones
├── kernelmain.c             # Entry point del kernel
├── link.ld                  # Script de enlace
├── Makefile                 # Compilacion
├── grub.cfg                 # Configuracion GRUB
├── include/                 # Cabeceras (.h)
│   ├── types.h              # Tipos enteros exactos
│   ├── asm.h                # Instrucciones in/out (inline asm)
│   ├── io.h                 # E/S pantalla VGA
│   ├── gdt.h                # Tabla de Descriptores Globales
│   ├── interrupts.h         # IDT y manejadores de interrupcion
│   ├── serial.h             # Puerto serie (UART 16550)
│   ├── keyboard.h           # Driver de teclado PS/2
│   ├── util_lib.h           # strlen
│   ├── timer.h              # delay()
│   ├── big_text.h           # Glyphs, draw_box, draw_big_char
│   └── splash.h             # animate_splash()
├── lib/                     # Implementaciones (.c)
│   ├── io.c                 # Framebuffer, cursor, scroll
│   ├── gdt.c                # GDT, descriptores de segmento
│   ├── interrupts.c         # IDT, PIC 8259A, despacho de interrupciones
│   ├── serial.c             # Puerto serie (UART 16550)
│   ├── keyboard.c           # Driver de teclado PS/2
│   ├── util_lib.c           # strlen
│   ├── timer.c              # delay() busy-wait
│   ├── big_text.c           # Glyphs, draw_box, draw_big_char
│   └── splash.c             # Animacion de bienvenida
├── build/                   # Objetos compilados
├── iso/                     # Imagen ISO
└── DemOS.iso                # ISO booteable final
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
| [VGA y Framebuffer](vga-framebuffer.md) | Como se escribe en pantalla |
| [Flujo de ejecucion](flujo-ejecucion.md) | Recorrido completo del codigo |
