# VGA y Framebuffer (io.c / io.h)

## ¿Que es el framebuffer VGA?

El **framebuffer VGA** es un area de memoria en la direccion fisica `0xB8000` que el hardware VGA mapea directamente a la pantalla en modo texto. Escribir en esta memoria produce texto visible al instante.

## Modo texto VGA

En el modo texto estandar (80x25):

| Propiedad | Valor |
|-----------|-------|
| Columnas | 80 |
| Filas | 25 |
| Direccion base | `0xB8000` |
| Bytes por celda | 2 (1 caracter + 1 atributo) |
| Total pantalla | 80 x 25 x 2 = 4000 bytes |

## Formato de cada celda

Cada posicion en la pantalla ocupa **2 bytes**:

```
+--------+--------+
| CHAR   | ATTR   |
+--------+--------+
byte 0   byte 1
```

### Byte 0 - Caracter (ASCII)

El codigo ASCII del caracter a mostrar. Ej: `'W'` = `0x57`.

### Byte 1 - Atributo (color)

```
Bit:    7   6   5   4  |  3   2   1   0
       +-------+-------+-------+-------+
       |   BLINK   |  BG COLOR | FG COLOR |
       +-------+-------+-------+-------+
```

| Bits | Significado |
|------|-------------|
| 0-3 | Color de frente (foreground) |
| 4-6 | Color de fondo (background) |
| 7 | Parpadeo (blink) o brillo intenso (depende del modo) |

### Colores disponibles

| Codigo | Color | Codigo | Color |
|--------|-------|--------|-------|
| 0x0 | Negro | 0x8 | Gris oscuro |
| 0x1 | Azul | 0x9 | Azul claro |
| 0x2 | Verde | 0xA | Verde claro |
| 0x3 | Cian | 0xB | Cian claro |
| 0x4 | Rojo | 0xC | Rojo claro |
| 0x5 | Magenta | 0xD | Magenta claro |
| 0x6 | Marron | 0xE | Marron claro |
| 0x7 | Gris claro | 0xF | Blanco |

## io.h - Definiciones y constantes

```c
#define FRAMEBUFFER 0x000B8000
#define CRTC_CMD_PORT   0x3D4
#define CRTC_DATA_PORT  0x3D5
```

### Puertos CRTC (Cathode Ray Tube Controller)

Controlan el cursor y el desplazamiento de pantalla:

| Puerto | Proposito |
|--------|-----------|
| `0x3D4` | Registro de comando (selecciona que registro del CRTC modificar) |
| `0x3D5` | Registro de datos (lee/escribe el valor del registro seleccionado) |

### Registros CRTC usados

| Comando | Significado |
|---------|-------------|
| `0x0E` | Byte alto de la posicion del cursor |
| `0x0F` | Byte bajo de la posicion del cursor |
| `0x0C` | Byte alto del inicio de pantalla (scroll) |
| `0x0D` | Byte bajo del inicio de pantalla (scroll) |

## io.c - Implementacion

### `write_letter_to_buffer()`

```c
void write_letter_to_buffer(uint8_t letter, uint16_t row, uint16_t col,
                            uint8_t fg_color, uint8_t bg_color)
{
    volatile uint16_t *framebuffer = (volatile uint16_t*) FRAMEBUFFER;

    uint16_t attribute = (bg_color << 4) | (fg_color & 0x0F);
    uint16_t character_with_attribute = (attribute << 8) | (letter & 0x00FF);
    uint16_t position = row * 80 + col;
    framebuffer[position] = character_with_attribute;
}
```

**Paso a paso:**

1. Obtiene un puntero al framebuffer (`0xB8000`).
2. **`volatile`**: Evita que el compilador optimice las escrituras (el hardware debe ver cada escritura).
3. Calcula el atributo combinando fondo y frente: `(bg << 4) | fg`.
4. Combina atributo y caracter: `(attribute << 8) | letter`.
5. Calcula la posicion lineal: `fila * 80 + columna`.
6. Escribe en el framebuffer.

### `move_cursor()`

```c
void move_cursor(uint16_t pos)
{
    uint16_t pos_low_byte = pos & 0x00FF;
    uint16_t pos_high_byte = (pos >> 8) & 0x00FF;

    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_high_byte);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, pos_low_byte);
}
```

Usa el puerto CRTC para mover el cursor a una posicion especifica (0-1999 para 80x25).

### `scroll()`

```c
void scroll(uint16_t row)
{
    uint16_t pos = 80 * row;
    // ... escribe byte alto y bajo en SCREEN_START_POS
}
```

Desplaza la pantalla hacia arriba ajustando el registro de inicio de pantalla del CRTC.

### `write_to_screen()`

```c
void write_to_screen(const char *buf, uint16_t len)
{
    for (uint32_t i = 0; i < len; i++)
        write_letter_to_buffer(buf[i], 0, i, WHITE, BLACK);
    move_cursor(len);
}
```

Escribe cada caracter del buffer en la **fila 0** con texto blanco sobre fondo negro, y mueve el cursor al final.

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Librerias del sistema](librerias.md) | [Flujo de ejecucion](flujo-ejecucion.md) |

| Relacionados |
|--------------|
| [Kernel principal](kernel-principal.md) |
| [Librerias del sistema](librerias.md) |
