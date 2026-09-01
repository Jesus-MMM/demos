---
title: "Driver de teclado"
order: 2
---

# Driver de teclado PS/2 (include/drivers/keyboard.h / src/drivers/keyboard.c)

## Que es el driver de teclado?

El **driver de teclado** maneja las interrupciones generadas por el teclado PS/2 (IRQ 1, interrupcion `0x21`), lee los **scancodes** del puerto de datos `0x60`, los traduce a caracteres ASCII usando el layout **QWERTY Latinoamericano** y los escribe en la pantalla VGA.

## El controlador PS/2

El **controlador PS/2** (Intel 8042 o compatible) gestiona la comunicacion entre el teclado y el CPU. DemOS lo configura para generar interrupciones cuando se presiona una tecla.

### Puertos del controlador

| Puerto | Nombre | Proposito |
|--------|--------|-----------|
| `0x60` | Data Port | Lee scancodes del teclado / escribe comandos de datos |
| `0x64` | Command/Status | Lee estado del controlador / escribe comandos de control |

### Registros de estado

| Bit | Nombre | Significado |
|-----|--------|-------------|
| 0 | OBF | Output Buffer Full - hay datos disponibles para leer |
| 1 | IBF | Input Buffer Full - el controlador esta procesando un comando |
| 4 | TIME-OUT | Error de timeout |
| 5 | PARITY | Error de paridad |

### Comandos del controlador

| Comando | Puerto | Proposito |
|---------|--------|-----------|
| `0xAE` | `0x64` | Habilitar teclado PS/2 |
| `0xAD` | `0x64` | Deshabilitar teclado PS/2 |
| `0x20` | `0x64` | Leer byte de configuracion del controlador |
| `0x60` | `0x64` | Escribir byte de configuracion del controlador |

### Byte de configuracion del controlador

| Bit | Nombre | Significado |
|-----|--------|-------------|
| 0 | IRQEN0 | Habilitar IRQ 0 (Timer) desde el controlador |
| 1 | IRQEN1 | Habilitar IRQ 1 (Teclado) desde el controlador |
| 4 | CLKDIS | Deshabilitar clock del teclado |
| 5 | Mouse | Habilitar mouse PS/2 |

### Comando del teclado

| Comando | Puerto | Proposito |
|---------|--------|-----------|
| `0xF4` | `0x60` | Habilitar escaneo (empezar a enviar scancodes) |
| `0xF5` | `0x60` | Deshabilitar escaneo |

## Scancodes Set 1

El teclado PS/2 envia **scancodes** (codigos de tecla) cuando se presiona o suelta una tecla. DemOS usa el **Set 1** (el mas comun).

### Estructura de un scancode

| Valor | Significado |
|-------|-------------|
| `0x00 - 0x7F` | **Make code** - tecla presionada |
| `0x80 - 0xFF` | **Break code** - tecla soltada (make code `| 0x80`) |

DemOS solo procesa make codes (`< 0x80`); los break codes se ignoran.

### Tabla de traduccion (QWERTY Latam)

| Scancode | ASCII | Tecla | Scancode | ASCII | Tecla |
|----------|-------|-------|----------|-------|-------|
| `0x02` | `1` | 1 | `0x10` | `q` | Q |
| `0x03` | `2` | 2 | `0x11` | `w` | W |
| `0x04` | `3` | 3 | `0x12` | `e` | E |
| `0x05` | `4` | 4 | `0x13` | `r` | R |
| `0x06` | `5` | 5 | `0x14` | `t` | T |
| `0x07` | `6` | 6 | `0x15` | `y` | Y |
| `0x08` | `7` | 7 | `0x16` | `u` | U |
| `0x09` | `8` | 8 | `0x17` | `i` | I |
| `0x0A` | `9` | 9 | `0x18` | `o` | O |
| `0x0B` | `0` | 0 | `0x19` | `p` | P |
| `0x1E` | `a` | A | `0x2C` | `z` | Z |
| `0x1F` | `s` | S | `0x2D` | `x` | X |
| `0x20` | `d` | D | `0x2E` | `c` | C |
| `0x21` | `f` | F | `0x2F` | `v` | V |
| `0x22` | `g` | G | `0x30` | `b` | B |
| `0x23` | `h` | H | `0x31` | `n` | N |
| `0x24` | `j` | J | `0x32` | `m` | M |
| `0x25` | `k` | K | `0x33` | `,` | , |
| `0x26` | `l` | L | `0x34` | `.` | . |
| `0x27` | `ñ` | Ñ | `0x35` | `-` | - |
| `0x1C` | `\n` | Enter | `0x0E` | `\b` | Backspace |
| `0x39` | ` ` | Space | | | |

## keyboard.h - API

```c
void keyboard_init(void);
uint32_t keyboard_handler(uint32_t esp);
void keyboard_set_cursor(uint16_t row, uint16_t col);
```

| Funcion | Proposito |
|---------|-----------|
| `keyboard_init()` | Configura el controlador PS/2 y habilita el escaneo |
| `keyboard_handler()` | Manejador de IRQ 1, llamado desde `handle_interrupt()` |
| `keyboard_set_cursor()` | Establece la posicion del cursor de escritura en pantalla |

## src/drivers/keyboard.c - Implementacion

### Variables globales

```c
static uint16_t cursor_row = 0;   // Fila actual del cursor (0-24)
static uint16_t cursor_col = 0;   // Columna actual del cursor (0-79)
```

El cursor de escritura se mantiene en memoria para saber donde escribir el siguiente caracter.

### `keyboard_init()` - Activacion del teclado

```c
void keyboard_init(void)
{
    while (inb(KB_COMMAND_PORT) & 0x1)
        inb(KB_DATA_PORT);

    outb(KB_COMMAND_PORT, 0xAE);         // Activar teclado PS/2

    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_COMMAND_PORT, 0x20);         // Leer configuracion del controlador
    while (!(inb(KB_COMMAND_PORT) & 0x1)) {}
    uint8_t status = (inb(KB_DATA_PORT) | 1) & ~0x10;
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_COMMAND_PORT, 0x60);         // Escribir nueva configuracion
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_DATA_PORT, status);
    while (inb(KB_COMMAND_PORT) & 0x2) {}
    outb(KB_DATA_PORT, 0xF4);            // Habilitar escaneo
}
```

**Paso a paso:**

| Paso | Que hace | Por que |
|------|----------|---------|
| 1. Drenar buffer | Leer y descartar datos pendientes | Evitar scancodes residuales |
| 2. `0xAE` al `0x64` | Habilitar el teclado | Asegurar que el teclado este activo |
| 3. Esperar IBF clear | Verificar que el controlador no esta ocupado | Evitar perder comandos |
| 4. `0x20` al `0x64` | Leer byte de configuracion | Obtener el estado actual |
| 5. Esperar OBF set | Verificar que el byte de configuracion esta disponible | Leer datos validos |
| 6. `\| 1` y `& ~0x10` | Bit 0 = 1 (habilitar IRQ0), bit 4 = 0 (habilitar clock) | Permitir que el teclado genere interrupciones |
| 7. Esperar IBF clear | Verificar que el controlador no esta ocupado | Evitar perder comandos |
| 8. `0x60` al `0x64` | Seleccionar registro de configuracion | Indicar que el siguiente dato es configuracion |
| 9. Esperar IBF clear | Verificar que el controlador limpie el registro | Confirmar seleccion |
| 10. `status` al `0x60` | Escribir nueva configuracion | Aplicar los cambios |
| 11. Esperar IBF clear | Verificar que el controlador no esta ocupado | Evitar perder el comando 0xF4 |
| 12. `0xF4` al `0x60` | Habilitar escaneo | El teclado empezara a enviar scancodes |

> **Nota**: Los guards IBF (bit 1 de `0x64`) y OBF (bit 0 de `0x64`) son criticos. Sin ellos, los comandos se pierden si el controlador esta ocupado procesando un dato anterior.

### `keyboard_handler()` - Manejo de interrupciones

```c
uint32_t keyboard_handler(uint32_t esp)
{
    uint8_t key = inb(KB_DATA_PORT);     // Leer scancode

    if (key >= 0x80)                      // Break code → ignorar
        return esp;

    char c = 0;

    switch (key) {
        case 0x02: c = '1'; break;
        // ... (tabla completa)
        case 0x27: c = (char)0xF1; break;   // ñ (CP-437)
        case 0x1C: c = '\n'; break;
        case 0x39: c = ' '; break;
        case 0x0E: c = '\b'; break;
        default:
            break;                          // Scancode desconocido → ignorar
    }

    if (c != 0)
        keyboard_putchar(c);

    return esp;
}
```

**Flujo:**

```
keyboard_handler(esp)
    │
    ├─ Leer scancode de 0x60
    ├─ Si >= 0x80: break code → ignorar, retornar esp
    ├─ Switch: traducir scancode a ASCII
    │   ├─ Si es tecla conocida: asignar caracter
    │   └─ Si es desconocida: ignorar (default: break)
    └─ Si c != 0: keyboard_putchar(c)
```

### `keyboard_putchar()` - Escritura en pantalla

```c
static void keyboard_putchar(char c)
{
    if (c == '\n') { keyboard_newline(); return; }
    if (c == '\b') { keyboard_backspace(); return; }

    write_letter_to_buffer(c, cursor_row, cursor_col, WHITE, BLACK);
    cursor_col++;

    if (cursor_col >= SCREEN_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= SCREEN_ROWS) {
            cursor_row = SCREEN_ROWS - 1;
            scroll(1);
        }
    }

    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}
```

**Manejo de caracteres especiales:**

| Caracter | Accion |
|----------|--------|
| `\n` | Nueva linea: resetear columna, incrementar fila |
| `\b` | Backspace: retroceder una posicion, borrar caracter |
| Imprimible | Escribir en framebuffer, avanzar cursor |

### `keyboard_newline()` - Nueva linea

```c
static void keyboard_newline(void)
{
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= SCREEN_ROWS) {
        cursor_row = SCREEN_ROWS - 1;
        scroll(1);
    }
    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}
```

### `keyboard_backspace()` - Borrar caracter

```c
static void keyboard_backspace(void)
{
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = SCREEN_COLS - 1;
    }
    write_letter_to_buffer(' ', cursor_row, cursor_col, WHITE, BLACK);
    move_cursor((cursor_row * SCREEN_COLS) + cursor_col);
}
```

Si el cursor esta en la columna 0, retrocede a la columna 79 de la fila anterior.

### Scancodes desconocidos

```c
default:
    break;
```

Los scancodes no reconocidos se ignoran silenciosamente (sin logging al serie para evitar latencia en la ruta de interrupcion).

## Flujo completo: tecla presionada

```
1. Usuario presiona tecla 'A'
2. Teclado PS/2 envia scancode 0x1E (make code)
3. PIC maestro genera IRQ 1
4. CPU ejecuta handle_interrupt_request0x01
5. handle_interrupt(0x21, esp) llama a keyboard_handler(esp)
6. keyboard_handler lee 0x1E del puerto 0x60
7. 0x1E < 0x80 → procesar
8. Switch case 0x1E → c = 'a'
9. keyboard_putchar('a')
10. write_letter_to_buffer('a', cursor_row, cursor_col, WHITE, BLACK)
11. cursor_col++
12. move_cursor(nueva_posicion)
13. handle_interrupt envia EOI al PIC
14. CPU reanuda codigo interrumpido
15. 'A' aparece en la pantalla VGA
```

## Dependencias

| Modulo | Funcion usada |
|--------|---------------|
| `asm.h` | `inb()`, `outb()` |
| `vga.h` | `write_letter_to_buffer()`, `move_cursor()`, `scroll()` |
| `serial.h` | `serial_write_string()`, `serial_write()` |
