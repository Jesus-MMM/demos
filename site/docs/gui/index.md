---
title: "GUI"
order: 5
---

# Interfaz Gráfica (GUI)

DemOS incluye una pequeña interfaz gráfica con ventanas, escritorio y widgets, dibujada sobre el `graphic_context_t` (abstracción del driver VGA en modo 13h).

Cuando el kernel cambia a modo gráfico (320x200x8), se construye el escritorio y se cargan las ventanas, junto con los callbacks del ratón y teclado. El disco contiene un fondo (`/FONDO.PNG`) que se muestra detrás de las ventanas.

## Arquitectura

```mermaid
graph TB
    subgraph "Núcleo GUI"
        GC[graphic_context_t<br/>deco del hardware]
        DESKTOP[desktop_t<br/>escritorio raíz]
        WINDOW[window_t<br/>ventana con barra]
        WIDGET[widget_t / composite_widget_t<br/>sistema base]
    end

    subgraph "Drivers"
        VGA[VGA Mode 13h]
        MOUSE[driver ratón]
        KBD[driver teclado]
    end

    subgraph "Hardware"
        SCREEN[Pantalla 320x200]
    end

    GC --> VGA
    DESKTOP --> WIDGET
    WINDOW --> WIDGET
    DESKTOP -->|callbacks| MOUSE
    DESKTOP -->|callbacks| KBD
    VGA --> SCREEN
```

## Flujo de dibujo por frame

```mermaid
flowchart LR
    A[Cursor del ratón<br/>actualizado] --> B[desktop_draw]
    B --> C[graphic_context_blit_image<br/>fondo persistente]
    C --> D[composite_widget_draw<br/>ventanas + hijos]
    D --> E[Dibujar rectángulo cursor]
    E --> F[graphic_context_flush<br/>espera retrace + swap]
```

## Jerarquía de widgets

```mermaid
graph TB
    DESKTOP[desktop_t<br/>composite raíz<br/>pantalla completa]
    W1[window_t<br/>ventana 1]
    W2[window_t<br/>ventana 2]
    DUMMY[widgets hijos<br/>compuestos anidados]

    DESKTOP --> W1
    DESKTOP --> W2
    W1 --> DUMMY
    W2 --> DUMMY
```

## En esta sección

- [Ventanas y escritorio (overview)](./overview/)
- [Sistema de widgets](./widget/)
- [Escritorio](./desktop/)
- [Ventanas](./window/)
- [Contexto gráfico](./graphic-context/)
