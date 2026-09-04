---
title: "Utilidades"
order: 6
---

# Utilidades

Herramientas auxiliares del kernel: decodificación de imágenes, renderizado de texto y animaciones.

## Módulos

```mermaid
graph TB
    subgraph "Utilidades"
        PNG[png.c<br/>decodificación de PNG]
        BT[big_text.c<br/>texto grande 5x5]
        SPLASH[splash.c<br/>animación de inicio]
        LIB[util_lib.c<br/>funciones varias]
    end

    subgraph "Dependencias"
        VGA[VGA legacy<br/>modo texto]
        VGAG[VGA modo 13h]
        TIMER[timer.c]
        TYPES[types.h]
    end

    PNG --> VGAG
    BT --> VGA
    SPLASH --> BT
    SPLASH --> TIMER
    SPLASH --> VGA
    LIB --> TYPES
```

## En esta sección

- [Decodificador PNG](./png/) — lectura de imágenes para la GUI
- [Texto grande](./big-text/) — glifos 5x5 y recuadros
- [Splash](./splash/) — animación de arranque
- [Utilidades varias](./util-lib/) — funciones de ayuda
- [Tipos y E/S](./types-asm/) — tipos enteros y puertos de E/S
