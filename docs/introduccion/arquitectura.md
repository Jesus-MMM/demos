---
title: "Arquitectura general"
order: 3
---

# Arquitectura general de DemOS

## Capas del sistema

```mermaid
graph TB
    subgraph "Hardware"
        VGA_HW[VGA<br/>Modo texto 80x25<br/>Modo gráfico 320x200x8]
        PS2_HW[PS/2 Controller<br/>Teclado + Mouse]
        UART_HW[UART 16550<br/>Puerto serie]
        ATA_HW[ATA/IDE<br/>Disco]
        CPU_HW[x86 32-bit<br/>CPU]
    end

    subgraph "Boot"
        GRUB3[GRUB<br/>Bootloader]
        LOADER3[loader.s<br/>Multiboot + Stack]
    end

    subgraph "Kernel Core"
        MAIN3[kernel_main<br/>Orquestador]
        GDT3[GDT<br/>Segmentación]
        IDT3[IDT + PIC<br/>Interrupciones]
        DM3[driver_manager<br/>Despacho por IRQ]
    end

    subgraph "Drivers"
        KB3[keyboard PS/2]
        MS3[mouse PS/2]
        SERIAL3[serial UART]
        VGA_TEXT3[vga_legacy<br/>Texto]
        VGA_GFX3[vga<br/>Gráfico 320x200]
        ATA3[ata<br/>Disco PIO]
        PCI3[pci<br/>Enumeración]
        TIMER3[timer<br/>Busy-wait]
    end

    subgraph "Filesystem"
        VFS3[VFS<br/>Capa abstracta]
        FAT33[FAT32<br/>Driver completo]
    end

    subgraph "GUI"
        GC3[graphic_context<br/>Double buffer]
        WIDGET3[widget_t<br/>Jerarquía de widgets]
        DESKTOP3[desktop_t<br/>Escritorio raíz]
        WINDOW3[window_t<br/>Ventanas]
    end

    subgraph "Utilidades"
        PNG3[png<br/>Decodificador]
        BIG3[big_text<br/>Glyphs 5x5]
        SPLASH3[splash<br/>Animación]
    end

    GRUB3 --> LOADER3
    LOADER3 --> MAIN3
    MAIN3 --> GDT3
    MAIN3 --> IDT3
    MAIN3 --> DM3

    DM3 --> KB3
    DM3 --> MS3
    KB3 --> PS2_HW
    MS3 --> PS2_HW
    SERIAL3 --> UART_HW
    VGA_TEXT3 --> VGA_HW
    VGA_GFX3 --> VGA_HW
    ATA3 --> ATA_HW
    PCI3 -.-> DM3

    MAIN3 --> VFS3
    VFS3 --> FAT33
    FAT33 --> ATA3

    MAIN3 --> DESKTOP3
    DESKTOP3 --> WIDGET3
    DESKTOP3 --> GC3
    DESKTOP3 --> WINDOW3
    GC3 --> VGA_GFX3

    DESKTOP3 --> PNG3
    PNG3 --> VFS3
    DESKTOP3 --> SPLASH3
    SPLASH3 --> BIG3
    SPLASH3 --> VGA_TEXT3
    TIMER3 -.-> SPLASH3

    IDT3 -.-> DM3
```

## Modo de ejecución

DemOS opera en **32 bits (protegido)** desde el inicio. GRUB cambia el CPU a modo protegido antes de transferir el control al kernel.

### Modos de video soportados

| Modo | Resolución | Colores | Uso |
|------|-----------|---------|-----|
| **Texto 80x25** | 80 columnas × 25 filas | 16 colores (4 bits) | Arranque, splash, depuración |
| **Gráfico 320x200** | 320 × 200 píxeles | 256 colores (8 bits, paleta) | Escritorio, ventanas, PNG |

El kernel comienza en modo texto para el splash y luego cambia a modo gráfico para el escritorio.

## Espacio de direccionamiento

```mermaid
graph LR
    subgraph "Memoria virtual x86 (32 bits)"
        A["0x00000000<br/>Rango bajo"] --> B["0x00100000<br/>Kernel base (1 MB)"]
        B --> C["0x00101000<br/>.text (código)"]
        C --> D["0x00102000<br/>.rodata"]
        D --> E["0x00103000<br/>.data"]
        E --> F["0x00104000<br/>.bss (stack 4KB)"]
        F --> G["0x00200000<br/>Memoria libre"]
        G --> H["0x000B8000<br/>VGA framebuffer (texto)"]
        H --> I["0x000A0000<br/>VGA framebuffer (gráf.)"]
    end
```

- **Kernel cargado en `0x00100000`** (1 MB) — especificación Multiboot
- **Pila del kernel**: 4 KB reservados en BSS (crece hacia abajo)
- **VGA framebuffer texto**: `0xB8000` — modo texto 80x25
- **VGA framebuffer gráfico**: segmento `0xA0000` — modo 320x200x8

## Sistema de interrupciones

```mermaid
graph LR
    subgraph "Hardware"
        KB_IRQ[Teclado → IRQ 1]
        MS_IRQ[Mouse → IRQ 12]
        TIMER_IRQ[Timer → IRQ 0]
    end

    subgraph "PIC 8259A"
        MASTER[PIC Maestro<br/>0x20-0x27]
        SLAVE[PIC Esclavo<br/>0x28-0x2F]
    end

    subgraph "IDT"
        IDT_TAB[256 entradas]
    end

    subgraph "Kernel"
        HANDLE[handle_interrupt<br/>Despacho central]
        KB_HANDLER[keyboard_handler]
        MS_HANDLER[mouse_handler]
        DM_HANDLER[driver_manager<br/>IRQ lookup]
    end

    KB_IRQ --> MASTER
    MS_IRQ --> SLAVE
    SLAVE -->|IRQ 2| MASTER
    MASTER --> IDT_TAB
    IDT_TAB --> HANDLE
    HANDLE --> DM_HANDLER
    DM_HANDLER --> KB_HANDLER
    DM_HANDLER --> MS_HANDLER
```

## Flujo de datos

```mermaid
sequenceDiagram
    participant G as GRUB
    participant L as loader.s
    participant K as kernel_main
    participant D as driver_manager
    participant HW as Hardware

    G->>L: Carga kernel.elf, cambia a modo protegido
    L->>L: Configura ESP (pila 4KB)
    L->>K: call kernel_main()

    K->>K: serial_init() — UART 16550
    K->>K: init_gdt() — Segmentos
    K->>K: init_interrupt_manager() — IDT + PIC

    K->>D: driver_manager_init()
    K->>D: keyboard_driver_init() + mouse_driver_init()
    K->>D: select_drivers() — Escaneo PCI
    K->>D: driver_manager_activate_all()

    Note over K: Habilita interrupciones (sti)

    loop Bucle principal
        K->>K: desktop_draw() + graphic_context_flush()
        HW-->>K: IRQ 1 (teclado) → handle_interrupt → keyboard_handler
        HW-->>K: IRQ 12 (mouse) → handle_interrupt → mouse_handler
    end
```
