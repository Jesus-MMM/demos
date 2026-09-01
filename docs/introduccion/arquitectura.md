---
title: "Arquitectura general"
order: 3
---

# Arquitectura general de DemOS

## Capas del sistema

```
+---------------------------+
|       GRUB (bootloader)   |  Carga el kernel en memoria
+---------------------------+
| asm/loader.s (ensamblador)|  Configura stack, llama a C
+---------------------------+
| src/kernel/main.c (entry) |  Inicia GDT, IDT, teclado, splash
+---------------------------+
| GDT (src/kernel/gdt.c)    |  Segmentos de codigo y datos (32 bits)
+---------------------------+
| IDT / PIC (interrupts)    |  Tabla de interrupciones, PIC 8259A
+---------------------------+
| keyboard (keyboard.c)     |  Driver PS/2, scancodes → ASCII
+---------------------------+
| mouse (src/drivers/       |  Driver PS/2, paquetes de movimiento
|        mouse.c)           |
+---------------------------+
|    src/util/splash.c      |  Orquesta la animacion
+---------------------------+
| big_text.c / timer.c      |  Letras grandes, delay, caja
+---------------------------+
|  vga.c / asm.h / util     |  Framebuffer VGA, puertos, util
+---------------------------+
| serial (src/drivers/      |  Puerto serie UART 16550 (debug)
|         serial.c)         |
+---------------------------+
|      Hardware (VGA/PS/2)  |  Pantalla, teclado, puertos E/S
+---------------------------+
```

## Modo de ejecucion

DemOS opera en **32 bits (protegido)** desde el inicio. GRUB cambia el CPU a modo protegido antes de transferir el control al kernel.

## Espacio de direccionamiento

- Kernel cargado en `0x00100000` (1 MB) — especificacion Multiboot.
- Pila del kernel: 4 KB reservados en BSS.

## Flujo de datos

1. **GRUB** lee `grub.cfg` y carga `kernel.elf`.
2. **asm/loader.s** configura el stack y llama a `kernel_main()`.
3. **kernel_main()** inicializa la **GDT** (segmentos de codigo/datos).
4. **kernel_main()** inicializa la **IDT** y el **PIC 8259A** (mapeo de IRQs).
5. **kernel_main()** inicializa el **driver del teclado PS/2** (IRQ 1).
6. **kernel_main()** desactiva el cursor y llama a `animate_splash()`.
7. **animate_splash()** usa `draw_box()` y `draw_big_char()` (de `src/util/big_text.c`) y `delay()` (de `src/drivers/timer.c`) para animar.
8. `draw_box()` y `draw_big_char()` escriben en el framebuffer VGA en `0xB8000`.
9. El **teclado** queda habilitado: cualquier tecla presionada genera IRQ 1 → `keyboard_handler()` → caracter en pantalla.
