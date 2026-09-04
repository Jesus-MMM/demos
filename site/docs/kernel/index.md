---
title: "Kernel"
order: 1
---

# Kernel

El núcleo de DemOS: desde el punto de entrada en ensamblador hasta el arranque de C, la gestión de memoria con la GDT, el sistema de interrupciones (IDT/PIC), el administrador centralizado de drivers y las librerías propias del sistema.

## Resumen

```mermaid
graph TB
    subgraph "Núcleo del kernel"
        LOADER2[loader.s<br/>Punto de entrada]
        MAIN2[kernel_main<br/>Orquestador C]
        GDT2[gdt.c<br/>Segmentos]
        IDT2[interrupts.c<br/>IDT + PIC]
        DM2[driver.c<br/>Driver manager]
        PCI2[pci.c<br/>Detección de dispositivos]
    end

    LOADER2 --> MAIN2
    MAIN2 --> GDT2
    MAIN2 --> IDT2
    MAIN2 --> DM2
    MAIN2 --> PCI2
    IDT2 --> DM2
```

## En esta sección

- [Loader en ensamblador](./loader-ensamblador/) — punto de entrada real y cabecera Multiboot
- [Script de enlace](./linker-script/) — organización de secciones en memoria
- [Kernel principal](./kernel-principal/) — orquestador en C (`src/kernel/main.c`)
- [GDT](./gdt/) — Tabla de Descriptores Globales
- [Sistema de interrupciones](./interrupts/) — IDT, PIC 8259A y stubs en ensamblador
- [Administrador de drivers](./driver-manager/) — despacho centralizado de IRQs
- [Librerías del sistema](./librerias/) — tipos, E/S, timer, big_text y splash
