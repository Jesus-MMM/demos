# Arquitectura general de DemOS

## Capas del sistema

```
+---------------------------+
|       GRUB (bootloader)   |  Carga el kernel en memoria
+---------------------------+
|   loader.s (ensamblador)  |  Configura stack, llama a C
+---------------------------+
|    kernelmain.c (entry)   |  Desactiva cursor + llama splash
+---------------------------+
|       splash.c            |  Orquesta la animacion
+---------------------------+
|   big_text.c / timer.c    |  Letras grandes, delay, caja
+---------------------------+
|    io.c / asm.h / util    |  Framebuffer VGA, puertos, util
+---------------------------+
|      Hardware (VGA)       |  Pantalla, puertos E/S
+---------------------------+
```

## Modo de ejecucion

DemOS opera en **32 bits (protegido)** desde el inicio. GRUB cambia el CPU a modo protegido antes de transferir el control al kernel.

## Espacio de direccionamiento

- Kernel cargado en `0x00100000` (1 MB) — especificacion Multiboot.
- Pila del kernel: 4 KB reservados en BSS.

## Flujo de datos

1. **GRUB** lee `grub.cfg` y carga `kernel.elf`.
2. **loader.s** configura el stack y llama a `kernel_main()`.
3. **kernel_main()** desactiva el cursor y llama a `animate_splash()`.
4. **animate_splash()** usa `draw_box()` y `draw_big_char()` (de `big_text.c`) y `delay()` (de `timer.c`) para animar.
5. `draw_box()` y `draw_big_char()` escriben en el framebuffer VGA en `0xB8000`.

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Introduccion](introduccion.md) | [Entorno y herramientas](entorno-setup.md) |

| Relacionados |
|--------------|
| [Loader en ensamblador](loader-ensamblador.md) |
| [Kernel principal](kernel-principal.md) |
| [Librerias del sistema](librerias.md) |
