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
loader.s (ensamblador)
    |  Configura la pila (ESP)
    |  Llama a kernel_main()
    v
kernelmain.c (entry point)
    |  serial_init()          → inicializa UART 16550
    |  init_gdt()             → configura segmentos de memoria
    |  init_interrupt_manager() → configura IDT + PIC 8259A
    |  keyboard_init()        → activa driver PS/2
    |  style_cursor(DISABLE)  → oculta cursor
    |  animate_splash()       → animacion de bienvenida
    v
splash.c (animacion)
    |  draw_box() → caja centrada con bordes dobles
    |  Por cada letra en "DemOS":
    |    draw_big_char() en DARKGREY → GREEN → LIGHTGREEN (con delay)
    |  Pulso colectivo 2 veces (GREEN ↔ LIGHTGREEN)
    v
big_text.c (render)
    |  draw_box(): escribe caracteres CP-437 (╔═╗║╚═╝) en framebuffer
    |  draw_big_char(): escribe █ o espacio segun el glyph 5x5
    v
io.c / framebuffer VGA
    |  write_letter_to_buffer() escribe en 0xB8000
    v
PANTALLA VGA
    |  Caja con "DemOS" en letras grandes, verde brillante
    v
keyboard listo
    |  Cualquier tecla genera IRQ 1
    |  keyboard_handler() escribe el caracter en pantalla
    v
BUCLE INFINITO (loader.s .hang)
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

### 3. loader.s (Assembly)

```nasm
loader:
    mov esp, kernel_stack + 4096    ; Pila al final del buffer de 4KB
    call kernel_main                 ; Salta a C
.hang:
    jmp .hang                        ; Bucle infinito
```

### 4. kernel_main.c (C)

```c
int kernel_main()
{
    serial_init(COM1_BASE_ADDRESS);    // Inicializar UART
    init_gdt(&gdt);                    // Configurar segmentos
    init_interrupt_manager(&gdt);      // Configurar IDT + PIC

    style_cursor(DISABLE);             // Oculta cursor parpadeante
    animate_splash();                  // Anima la pantalla de bienvenida

    keyboard_set_cursor(0, 0);         // Cursor en esquina superior izquierda
    style_cursor(SMALL);               // Habilitar cursor visible
    keyboard_init();                   // Activar driver PS/2
    return 0;
}
```

### 5. splash.c — Animacion

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

### 6. big_text.c — Renderizado

**draw_box()** escribe estos caracteres en el framebuffer:

```
Col: 21   22   23  ...  56   57
     ╔    ═    ═   ...  ═    ╗    ← fila 8
     ║    (espacios)    ║    ← filas 9-15
     ╚    ═    ═   ...  ═    ╝    ← fila 16
```

**draw_big_char()** recorre la matriz 5x5 del glyph y escribe `█` (0xDB) donde hay un 1, y espacio donde hay un 0.

Por ejemplo, `glyph_D[0]` = `{1,1,1,1,0}` produce `████` en la fila 10, columnas 25-29.

### 7. io.c — Framebuffer VGA

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
CPU: bucle infinito en loader.s:.hang, interrupciones activas
```

## Resumen de tecnologias involucradas

| Tecnologia | Uso en DemOS |
|------------|--------------|
| **x86 Assembly (NASM)** | Cabecera Multiboot, configuracion de pila, interrupt stubs |
| **C (GCC)** | Logica del kernel, modulos splash/big_text/timer/io/keyboard/serial |
| **GRUB** | Gestor de arranque |
| **VGA Text Mode (CP-437)** | Caracteres, bloques, bordes dobles en pantalla |
| **GDT** | Segmentos de memoria en modo protegido |
| **IDT / PIC 8259A** | Manejo de interrupciones y excepciones |
| **UART 16550** | Puerto serie para depuracion |
| **PS/2 Controller** | Driver de teclado (scancodes → ASCII) |
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
