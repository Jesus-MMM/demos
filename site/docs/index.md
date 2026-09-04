---
title: "DemOS"
order: 1
---

# DemOS

Sistema operativo minimalista desarrollado desde cero con fines educativos.

**DemOS** es un proyecto que implementa un kernel x86 de 32 bits en **C** y **ensamblador (NASM)**, arrancado mediante **GRUB** con la especificación **Multiboot**. Incluye gestión de memoria (GDT/IDT), controlador PIC 8259A, drivers PS/2 de teclado y mouse, puerto serie UART 16550, enumeración PCI, sistema de archivos VFS con soporte FAT32, decodificador PNG, y un entorno gráfico de ventanas con double-buffering.

## Arquitectura del sistema

```mermaid
graph TB
    subgraph Aplicación
        SPLASH[animate_splash<br/>Animación de arranque]
        DESKTOP[desktop_t<br/>Escritorio gráfico]
        WINDOWS[window_t<br/>Ventanas arrastrables]
    end

    subgraph GUI
        GC[graphic_context<br/>Double buffer + flush]
        WIDGET[widget_t<br/>Sistema de widgets]
    end

    subgraph Utilidades
        PNG[png_decode_indexed<br/>Decodificador PNG]
        BIGTEXT[big_text<br/>Glyphs 5x5 + cajas]
        SPLASH2[splash<br/>Layout animado]
        LIB[util_lib<br/>strlen]
    end

    subgraph Filesystem
        VFS[VFS<br/>Capa de abstracción]
        FAT32[FAT32<br/>Driver de disco]
        ATA[ATA/IDE<br/>Driver de disco]
    end

    subgraph Drivers
        KB[keyboard PS/2<br/>IRQ 0x21]
        MS[mouse PS/2<br/>IRQ 0x2C]
        VGA_TEXT[vga_legacy<br/>Modo texto 80x25]
        VGA_GFX[vga<br/>Modo 320x200x8]
        SERIAL[serial<br/>UART 16550]
        TIMER[timer<br/>delay busy-wait]
        PCI[PCI<br/>Enumeración bus]
    end

    subgraph Kernel
        MAIN[kernel_main<br/>Entry point C]
        GDT[GDT<br/>Descriptores de segmento]
        IDT[IDT + PIC<br/>Manejo de interrupciones]
        DM[driver_manager<br/>Administrador de drivers]
        LOADER[loader.s<br/>Arranque Multiboot]
    end

    subgraph Hardware
        CPU[x86 32-bit CPU]
        GRUB2[GRUB bootloader]
        PS2[PS/2 Controller]
        UART[UART 16550]
        DISK[Disco ATA/IDE]
    end

    GRUB2 --> LOADER
    LOADER --> MAIN
    MAIN --> GDT
    MAIN --> IDT
    MAIN --> DM
    DM --> KB
    DM --> MS
    KB --> PS2
    MS --> PS2
    SERIAL --> UART
    SERIAL --> VGA_TEXT
    ATA --> DISK

    MAIN --> VGA_GFX
    MAIN --> DESKTOP
    DESKTOP --> GC
    DESKTOP --> WINDOWS
    GC --> VGA_GFX
    VFS --> FAT32
    FAT32 --> ATA
    MAIN --> VFS
    PNG --> VFS
    DESKTOP --> PNG

    IDT -.-> DM
    DM -.-> KB
    DM -.-> MS
```

## Cómo se organiza esta documentación

| Sección | Contenido |
|---------|-----------|
| [Introducción](/introduccion/) | Qué es DemOS, entorno de desarrollo, arquitectura general y flujo de ejecución |
| [Kernel](/kernel/) | Loader, linker script, kernel principal, GDT, interrupciones, administrador de drivers y librerías |
| [Drivers](/drivers/) | VGA (texto y gráfico), teclado, mouse, puerto serie, temporizador, ATA/IDE y controlador PCI |
| [Filesystem](/filesystem/) | Sistema de archivos VFS y driver FAT32 |
| [GUI](/gui/) | Sistema de widgets, escritorio, ventanas y contexto gráfico |
| [Utilidades](/utilidades/) | Decodificador PNG, glyphs grandes, splash y tipos base |
| [Build](/build/) | Sistema de compilación con Makefile y generación de ISO/disco |

## Empezar

Consulta la [introducción](/introduccion/) para conocer el proyecto, o dirígete directamente al [entorno y herramientas](/introduccion/entorno-setup/) para compilar y ejecutar DemOS.
