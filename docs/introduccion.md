# Introduccion a DemOS

**DemOS** es un sistema operativo minimalista desarrollado desde cero con fines educativos.

## Que hace DemOS?

Al iniciarse, DemOS:

1. Carga el kernel mediante **GRUB** usando la especificacion **Multiboot**.
2. Inicializa la pila (stack) del kernel.
3. Desactiva el cursor parpadeante.
4. Dibuja una **caja** centrada en la pantalla con bordes dobles.
5. Anima el texto **"DemOS"** en letras grandes (5x5) dentro de la caja, con transiciones de color verde.
6. Se detiene en un bucle infinito.

## Estructura del proyecto

```
DemOS/
├── loader.s                 # Arranque (Multiboot + stack)
├── kernelmain.c             # Entry point del kernel
├── link.ld                  # Script de enlace
├── Makefile                 # Compilacion
├── grub.cfg                 # Configuracion GRUB
├── include/                 # Cabeceras (.h)
│   ├── types.h              # Tipos enteros exactos
│   ├── asm.h                # Instrucciones in/out (inline asm)
│   ├── io.h                 # E/S pantalla VGA
│   ├── util_lib.h           # strlen
│   ├── timer.h              # delay()
│   ├── big_text.h           # Glyphs, draw_box, draw_big_char
│   └── splash.h             # animate_splash()
├── lib/                     # Implementaciones (.c)
│   ├── io.c                 # Framebuffer, cursor, scroll
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
| [VGA y Framebuffer](vga-framebuffer.md) | Como se escribe en pantalla |
| [Flujo de ejecucion](flujo-ejecucion.md) | Recorrido completo del codigo |
