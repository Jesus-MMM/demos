---
title: "Flujo de ejecución"
order: 4
---

# Flujo de ejecución completo

Este documento traza el recorrido del código desde que el CPU se enciende hasta que el escritorio gráfico aparece en pantalla.

## Diagrama de secuencia completo

```mermaid
sequenceDiagram
    participant BIOS as BIOS/Firmware
    participant GRUB as GRUB
    participant ASM as loader.s
    participant C as kernel_main()
    participant GDT as GDT
    participant IDT as IDT + PIC
    participant DM as driver_manager
    participant KB as keyboard
    participant MS as mouse
    participant PCI as PCI scan
    participant VGA as VGA (Mode 13h)
    participant VFS as VFS + FAT32
    participant PNG as PNG decoder
    participant GUI as Desktop + Windows

    BIOS->>GRUB: POST, buscar dispositivo booteable
    GRUB->>GRUB: Lee grub.cfg, busca cabecera Multiboot
    GRUB->>ASM: Salta a loader (modo protegido 32-bit)

    ASM->>ASM: mov esp, kernel_stack + 4096
    ASM->>C: call kernel_main()

    C->>C: serial_init(COM1) — UART 16550
    C->>GDT: init_gdt() — 4 descriptores
    GDT-->>C: LGDT + ljmp (recargar CS)

    C->>IDT: init_interrupt_manager() — 256 entradas
    C->>IDT: PIC ICW1-ICW4 (remapeo IRQs)
    Note over IDT: IRQ 0→0x20, IRQ 1→0x21, IRQ 12→0x2C

    C->>DM: driver_manager_init()
    C->>DM: keyboard_driver_init(kb_driver)
    C->>DM: mouse_driver_init(ms_driver)
    C->>DM: driver_manager_add(kb_driver)
    C->>DM: driver_manager_add(ms_driver)

    C->>PCI: select_drivers() — escaneo 256 buses × 32 devices
    PCI-->>DM: driver_manager_add() por cada dispositivo

    C->>DM: driver_manager_activate_all()
    DM->>KB: keyboard_activate() — drenar buffer, 0xF4
    DM->>MS: mouse_activate() — habilitar puerto 2, 0xF4

    C->>VGA: vga_set_mode(320, 200, 8)
    VGA-->>VGA: Escribir registros Mode 13h

    C->>VFS: fs_test_run() — montar FAT32
    VFS->>VFS: ATA read sectors → parse BPB → montar /
    C->>PNG: background_load_from_disk()
    PNG->>VFS: vfs_open("/FONDO.PNG")
    PNG->>PNG: png_decode_indexed() → background_pixels
    C->>GUI: desktop_set_background(pixels, w, h)

    C->>GUI: window_init(win1) + window_init(win2)
    C->>GUI: composite_widget_add_child() × 2

    Note over C: asm volatile("sti") — habilitar interrupciones

    loop Bucle principal (for-ever)
        C->>GUI: desktop_draw(&desktop, &gc)
        GUI->>GUI: Composite: draw background → windows → cursor
        C->>VGA: graphic_context_flush() — back buffer → framebuffer
        Note over VGA: Espera retrace vertical (vsync)
    end
```

## Paso a paso detallado

### 1. Encendido / Reset

- CPU comienza ejecutando la BIOS en `0xFFFFFFF0`.
- POST (Power-On Self Test) y búsqueda de dispositivo booteable.

### 2. GRUB (bootloader)

- GRUB lee `grub.cfg`:
  ```cfg
  set timeout=5
  set default=0

  menuentry "DemOS" {
      multiboot /boot/kernel.elf
      boot
  }
  ```
- Localiza la cabecera Multiboot en `0x00100000`:
  - Magic: `0x1BADB002`, Flags: `0x0`, Checksum: `-(0x1BADB002)`.
- Cambia a **modo protegido 32 bits** y salta a `loader`.

### 3. asm/loader.s (Assembly)

```nasm
loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE  ; Pila al final del buffer de 4KB
    call kernel_main                           ; Salta a C
.hang:
    jmp .hang                                  ; Bucle infinito
```

### 4. src/kernel/main.c (C)

El punto de entrada en C orquesta toda la inicialización:

```c
int kernel_main()
{
    static global_descriptor_table gdt;

    serial_init(COM1_BASE_ADDRESS);            // UART para depuración
    init_gdt(&gdt);                            // Segmentos de memoria
    init_interrupt_manager(&gdt);              // IDT + PIC 8259A

    driver_manager_init(&global_driver_manager);

    // Modo gráfico: el escritorio recibe eventos
    graphic_context_init(&gc);
    desktop_init(&desktop, &gc);

    keyboard_driver_init(&kb_driver, desktop_on_key_down, &desktop);
    kb_driver.on_key_up = desktop_on_key_up;
    driver_manager_add(&global_driver_manager, &kb_driver.base);

    mouse_driver_init(&ms_driver, desktop_on_mouse_move, &desktop);
    ms_driver.on_mouse_button = desktop_on_mouse_button;
    driver_manager_add(&global_driver_manager, &ms_driver.base);

    select_drivers(&global_driver_manager);    // Escaneo PCI
    driver_manager_activate_all(&global_driver_manager);

    fs_test_run();                             // Montar FAT32

    vga_set_mode(320, 200, 8);                // Cambiar a modo gráfico
    vga_fill_rectangle(0, 0, 320, 200, 0x09);
    background_load_from_disk();               // Cargar PNG de fondo

    // Crear ventanas de ejemplo
    window_init(&win1, &desktop.base.base, 10, 10, 20, 20, 0x04, NULL);
    composite_widget_add_child(&desktop.base, &win1.base.base);
    window_init(&win2, &desktop.base.base, 40, 15, 30, 30, 0x02, NULL);
    composite_widget_add_child(&desktop.base, &win2.base.base);

    asm volatile("sti");                       // Habilitar interrupciones

    for (;;) {
        desktop_draw(&desktop, &gc);           // Dibujar todo
        graphic_context_flush(&gc);            // Copiar a pantalla
    }
    return 0;
}
```

### 5. Inicialización de drivers

```mermaid
flowchart TD
    A[driver_manager_init] --> B[keyboard_driver_init]
    A --> C[mouse_driver_init]
    B --> D[driver_manager_add kb]
    C --> E[driver_manager_add ms]
    D --> F[select_drivers - PCI scan]
    E --> F
    F --> G[driver_manager_activate_all]
    G --> H[keyboard_activate - PS/2 init]
    G --> I[mouse_activate - PS/2 init]
    H --> J[Teclado listo: IRQ 0x21]
    I --> K[Mouse listo: IRQ 0x2C]
```

Cada driver registra sus callbacks de interrupción. El `driver_manager` despacha IRQs al driver correcto usando `driver_manager_get_driver_for_irq()`.

### 6. Cambio a modo gráfico

Después de la inicialización de drivers, el kernel cambia al **modo gráfico 320x200x256** (Mode 13h) escribiendo los registros VGA apropiados. Luego carga el fondo PNG del disco FAT32 y crea las ventanas de ejemplo.

### 7. Bucle principal

```mermaid
flowchart LR
    A[desktop_draw] --> B[graphic_context_flush]
    B --> C[VGA framebuffer]
    C --> D[Pantalla]
    D --> E[ IRQs: keyboard/mouse ]
    E --> A
```

El kernel queda en un bucle infinito que:
1. **Dibuja** el escritorio (fondo + ventanas + cursor) en el back buffer
2. **Hace flush** del back buffer al framebuffer VGA (esperando vsync)
3. **Recibe interrupciones** de teclado y mouse que actualizan el estado

## Resumen de tecnologías involucradas

| Tecnología | Uso en DemOS |
|------------|--------------|
| **x86 Assembly (NASM)** | Cabecera Multiboot, configuración de pila, interrupt stubs |
| **C (GCC -m32)** | Lógica del kernel, drivers, GUI, filesystem, PNG |
| **GRUB** | Gestor de arranque Multiboot |
| **VGA Text Mode (CP-437)** | Modo texto 80x25 para splash y depuración |
| **VGA Mode 13h** | Modo gráfico 320x200x256 para el escritorio |
| **GDT** | Segmentos de memoria en modo protegido |
| **IDT / PIC 8259A** | Manejo de interrupciones y excepciones |
| **Driver Manager** | Despacho centralizado de IRQs con polimorfismo |
| **PS/2 Controller** | Driver de teclado y mouse (scancodes/paquetes → pantalla) |
| **UART 16550** | Puerto serie para depuración |
| **PCI** | Enumeración de dispositivos y detección de hardware |
| **ATA/IDE PIO** | Lectura de sectores del disco para FAT32 |
| **VFS + FAT32** | Sistema de archivos virtual con soporte FAT32 |
| **PNG decoder** | Decodificador completo desde cero (DEFLATE + filtros) |
| **GUI widgets** | Sistema de ventanas con double-buffering y vsync |
| **Framebuffer (0xB8000/0xA0000)** | Escritura directa en memoria de video |
| **Linker Script** | Organización de secciones en memoria |
| **Makefile** | Automatización de compilación + disco virtual |
| **QEMU** | Emulación para pruebas |
| **Multiboot spec** | Estándar de interfaz kernel-bootloader |
