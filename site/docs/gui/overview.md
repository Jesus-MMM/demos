---
title: "GUI: ventanas y escritorio"
order: 1
---

# GUI: ventanas y escritorio (visión general)

Esta página resume cómo se conectan el escritorio, las ventanas y los drivers para formar la interfaz gráfica de DemOS.

## Flujo de arranque de la GUI

```mermaid
sequenceDiagram
    participant Main as kernel_main
    participant VGA as VGA (Mode 13h)
    participant GC as graphic_context_t
    participant DESK as desktop_t
    participant DRV as Chips/Drivers
    participant FAT as VFS/FAT32
    participant PNG as PNG decoder

    Main->>VGA: init_vga (320x200x8)
    Main->>GC: graphic_context_init
    Main->>DESK: desktop_init
    Main->>DRV: registrar callbacks (ratón/teclado)
    Main->>PNG: decodificar /FONDO.PNG
    PNG->>FAT: vfs_open + read
    PNG-->>Main: fondo indexado
    Main->>DESK: desktop_set_background
    Main->>DESK: crear ventanas window_init
    loop Cada frame
        DESK->>DESK: desktop_draw (fondo + ventanas + cursor)
        DESK->>VGA: graphic_context_flush
    end
```

## Eventos de entrada

Los callbacks que recibe el escritorio provienen de los ISRs del ratón y teclado. Una vez el desktop los recibe, los re-despacha a la ventana/gestor bajo el cursor o con el foco:

```mermaid
graph LR
    subgraph "Hardware"
        IPS2[PS/2 ratón]
        ISER[Teclado]
    end
    subgraph "Drivers"
        M[isr33_mouse<br/>handler IRQ12]
        K[isr33_key<br/>handler IRQ1]
    end
    subgraph "Escritorio"
        MM[desktop_on_mouse_move]
        MB[desktop_on_mouse_button]
        KD[desktop_on_key_down]
    end
    subgraph "Ventanas"
        MW[window_on_mouse_*]
    end

    IPS2 --> M
    ISER --> K
    M --> MM
    K --> KD
    M --> MB
    MM --> MW
    MB --> MW
```

## Inicialización típica en `main.c`

```c
// (pseudocódigo)
graphic_context_init(&gc);
desktop_init(&desktop, &gc);
desktop_set_background(&desktop, fondo, 320, 200);

// Registrar callbacks del ratón y teclado...
chipset_register_irq(IRQ12, desktop_on_mouse_move, &desktop);
// ...

// Crear ventanas
window_init(&win1, &desktop.base, 20, 30, 200, 150, col, "Ventana 1");
composite_widget_add_child(&desktop.base, &win1.base);
```

## Documentos detallados

- [Sistema de widgets](./widget/) — base `widget_t` y componentes
- [Escritorio](./desktop/) — raíz a pantalla completa, fondo y cursor
- [Ventanas](./window/) — barras de título y arrastre
- [Contexto gráfico](./graphic-context/) — capa de dibujo sobre el VGA
