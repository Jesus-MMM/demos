---
title: "DemOS"
order: 1
---

# DemOS

Sistema operativo minimalista desarrollado desde cero con fines educativos.

**DemOS** es un proyecto que implementa un kernel x86 de 32 bits en **C** y **ensamblador (NASM)**, arrancado mediante **GRUB** con la especificación **Multiboot**. Incluye GDT, IDT, controlador PIC 8259A, drivers PS/2 de teclado y mouse, puerto serie UART 16550, enumeración PCI y salida por framebuffer VGA.

## Cómo se organiza esta documentación

| Sección | Contenido |
|---------|-----------|
| [Introducción](/introduccion/) | Qué es DemOS, entorno de desarrollo, arquitectura y flujo de ejecución |
| [Kernel](/kernel/) | Loader, linker script, kernel principal, GDT, interrupciones y librerías |
| [Drivers](/drivers/) | VGA, teclado, mouse, puerto serie y controlador PCI |
| [Build](/build/) | Sistema de compilación con Makefile |

## Empezar

Consulta la [introducción](/introduccion/) para conocer el proyecto, o dirígete directamente al [entorno y herramientas](/introduccion/entorno-setup/) para compilar y ejecutar DemOS.
