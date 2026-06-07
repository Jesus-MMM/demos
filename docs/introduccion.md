# Introduccion a DemOS

**DemOS** es un sistema operativo minimalista desarrollado desde cero con fines educativos. Su objetivo es demostrar los conceptos fundamentales detras del arranque y funcionamiento basico de un sistema operativo.

## ¿Que hace DemOS?

Al iniciarse, DemOS:

1. Carga el kernel mediante el gestor de arranque **GRUB** usando la especificación **Multiboot**.
2. Inicializa la pila (stack) del kernel.
3. Escribe el mensaje `"Welcome to DemOS"` en la pantalla usando el modo texto **VGA**.
4. Se detiene en un bucle infinito.

## Estructura del proyecto

```
DemOS/
├── loader.s            # Código ensamblador de arranque (Multiboot + stack)
├── kernelmain.c        # Kernel principal en C
├── link.ld             # Script de enlace (linker script)
├── Makefile            # Sistema de compilación
├── grub.cfg            # Configuración de GRUB
├── include/            # Archivos de cabecera (.h)
│   ├── types.h         # Definición de tipos enteros
│   ├── asm.h           # Instrucciones ensamblador inline (in/out)
│   ├── io.h            # Funciones de entrada/salida de pantalla
│   └── util_lib.h      # Utilidades (strlen)
├── lib/                # Implementaciones (.c)
│   ├── io.c            # Manejo de framebuffer VGA y cursor
│   └── util_lib.c      # Implementación de strlen
├── build/              # Archivos objeto compilados
├── iso/                # Imagen ISO generada
└── DemOS.iso           # ISO booteable final
```

## Navegación

| Documento | Descripcion |
|-----------|-------------|
| [Arquitectura general](arquitectura.md) | Vision general del sistema |
| [Entorno y herramientas](entorno-setup.md) | Requisitos para compilar y ejecutar |
| [Sistema de compilacion](makefile.md) | Explicacion del Makefile |
| [Loader en ensamblador](loader-ensamblador.md) | El punto de entrada del kernel |
| [Kernel principal](kernel-principal.md) | Corazon del sistema operativo |
| [Librerias del sistema](librerias.md) | Tipos, E/S, utilidades |
| [VGA y Framebuffer](vga-framebuffer.md) | Como se escribe en pantalla |
| [Flujo de ejecucion](flujo-ejecucion.md) | Recorrido completo del codigo |
