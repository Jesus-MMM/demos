# Flujo de ejecucion completo

Este documento traza el recorrido del codigo desde que el CPU se enciende hasta que la animacion aparece en pantalla.

## Diagrama de secuencia

```
ENCENDIDO
    |
    v
BIOS (firmware)
    |  Busca dispositivo booteable
    v
GRUB (bootloader)
    |  Lee grub.cfg, busca cabecera Multiboot en kernel.elf
    |  Cambia a modo protegido (32 bits)
    v
asm/loader.s (ensamblador)
    |  Configura la pila (ESP)
    |  Llama a kernel_main()
    v
src/kernel/main.c (entry point)
    |  serial_init()          → inicializa UART 16550
    |  init_gdt()             → configura segmentos de memoria
    |  init_interrupt_manager() → configura IDT + PIC 8259A
    |  driver_manager_init()  → inicializa el administrador de drivers
    |  select_drivers()       → escanea bus PCI, detecta dispositivos y registra drivers
    |  keyboard_init()        → activa driver de teclado PS/2
    |  mouse_init()           → activa driver de mouse PS/2
    |  asm sti()              → habilita interrupciones del CPU
    v
keyboard y mouse listos
    |  Tecla → IRQ 1 → keyboard_handler() → caracter en pantalla
    |  Mouse → IRQ 12 → mouse_handler() → cursor visual se mueve
    v
BUCLE INFINITO (asm/loader.s .hang)
```

## Paso a paso detallado

### 1. Encendido / Reset

- CPU comienza ejecutando la BIOS en `0xFFFFFFF0`.
- POST (Power-On Self Test) y busqueda de dispositivo booteable.

### 2. GRUB (bootloader)

- GRUB lee `grub.cfg`:
  ```cfg
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
    mov esp, kernel_stack + 4096    ; Pila al final del buffer de 4KB
    call kernel_main                 ; Salta a C
.hang:
    jmp .hang                        ; Bucle infinito
```

### 4. src/kernel/main.c (C)

 ```c
int kernel_main()
{
    static global_descriptor_table gdt;

    serial_init(COM1_BASE_ADDRESS);          // Inicializar UART
    init_gdt(&gdt);                          // Configurar segmentos
    init_interrupt_manager(&gdt);            // Configurar IDT + PIC

    driver_manager_init(&global_driver_manager);  // Inicializar administ. de drivers

    keyboard_driver_init(&kb_driver, keyboard_default_on_key_down, &kb_driver);
    driver_manager_add(&global_driver_manager, &kb_driver.base);

    mouse_driver_init(&ms_driver, mouse_default_on_move, &ms_driver);
    driver_manager_add(&global_driver_manager, &ms_driver.base);

    select_drivers(&global_driver_manager);  // Escanear bus PCI y registrar drivers

    driver_manager_activate_all(&global_driver_manager); // Activar drivers

    keyboard_set_cursor(0, 0);               // Cursor en esquina superior izquierda
    style_cursor(SMALL);                     // Habilitar cursor visible

    asm volatile("sti");                     // Habilitar interrupciones
    return 0;
}
```

### 5. src/util/splash.c — Animacion

```c
void animate_splash(void)
{
    // 1. Caja centrada (fila 8, col 21, 37x9)
    draw_box(21, 8, 37, 9, GREEN);

    // 2. Letra por letra con transicion de color
    for (i = 0; i < 5; i++)
    {
        lc = 25 + i * 6;
        draw_big_char(glyph[i], 10, lc, DARKGREY);   // fantasma
        delay(30000000);
        draw_big_char(glyph[i], 10, lc, GREEN);      // verde
        delay(15000000);
        draw_big_char(glyph[i], 10, lc, LIGHTGREEN); // brillante
        delay(15000000);
    }

    // 3. Pulso colectivo (2 ciclos)
    for (p = 0; p < 2; p++)
    {
        // Todas las letras en verde
        // delay
        // Todas en verde brillante
        // delay
    }
}
```

### 6. src/util/big_text.c — Renderizado

**draw_box()** escribe estos caracteres en el framebuffer:

```
Col: 21   22   23  ...  56   57
     ╔    ═    ═   ...  ═    ╗    ← fila 8
     ║    (espacios)    ║    ← filas 9-15
     ╚    ═    ═   ...  ═    ╝    ← fila 16
```

**draw_big_char()** recorre la matriz 5x5 del glyph y escribe `█` (0xDB) donde hay un 1, y espacio donde hay un 0.

Por ejemplo, `glyph_D[0]` = `{1,1,1,1,0}` produce `████` en la fila 10, columnas 25-29.

### 7. vga.c — Framebuffer VGA

Cada llamada a `write_letter_to_buffer()` calcula:

```
framebuffer[fila * 80 + columna] = (atributo << 8) | caracter
```

Donde `atributo = (fondo << 4) | frente`, que para verde sobre negro es `0x02`.

### 8. Estado final

```
         ┌─────────────────────────────────────┐
         │                                     │
         │   █████ █████ █   █ █████ █████     │
         │   █   █ █     ██ ██ █   █ █         │
         │   █   █ █████ █ █ █ █   █ █████     │
         │   █   █     █ █   █ █   █     █     │
         │   █████ █████ █   █ █████ █████     │
         │                                     │
         └─────────────────────────────────────┘

Cursor: SMALL (visible en fila 0, columna 0)
Keyboard: habilitado (IRQ 1 → keyboard_handler)
Mouse: habilitado (IRQ 12 → mouse_handler, cursor visual activo)
CPU: bucle infinito en asm/loader.s:.hang, interrupciones activas
```

## Resumen de tecnologias involucradas

| Tecnologia | Uso en DemOS |
|------------|--------------|
| **x86 Assembly (NASM)** | Cabecera Multiboot, configuracion de pila, interrupt stubs |
| **C (GCC)** | Logica del kernel, modulos splash/big_text/timer/vga/keyboard/serial |
| **GRUB** | Gestor de arranque |
| **VGA Text Mode (CP-437)** | Caracteres, bloques, bordes dobles en pantalla |
| **GDT** | Segmentos de memoria en modo protegido |
| **IDT / PIC 8259A** | Manejo de interrupciones y excepciones |
| **PCIe bus** | Enumeracion de dispositivos, lectura de BARs, seleccion de drivers |
| **UART 16550** | Puerto serie para depuracion |
| **PS/2 Controller** | Driver de teclado y mouse (scancodes/paquetes → pantalla) |
| **Puertos E/S (x86)** | Control de cursor y scroll via CRTC |
| **Framebuffer (0xB8000)** | Escritura directa en memoria de video |
| **Linker Script** | Organizacion de secciones en memoria |
| **Makefile** | Automatizacion de compilacion |
| **QEMU** | Emulacion para pruebas |
| **Multiboot spec** | Estandar de interfaz kernel-bootloader |

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [VGA y Framebuffer](vga-framebuffer.md) | [Volver al inicio](introduccion.md) |

---

**Fin de la documentacion de DemOS**
