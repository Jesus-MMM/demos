# VGA y Framebuffer (src/drivers/vga.c / include/drivers/vga.h)

## ¿Que es el framebuffer VGA?

El **framebuffer VGA** es un area de memoria en la direccion fisica `0xB8000` que el hardware VGA mapea directamente a la pantalla en modo texto. Escribir en esta memoria produce texto visible al instante.

## Modo texto VGA (80x25)

| Propiedad | Valor |
|-----------|-------|
| Columnas | 80 |
| Filas | 25 |
| Direccion base | `0xB8000` |
| Bytes por celda | 2 (1 caracter + 1 atributo) |
| Total pantalla | 80 x 25 x 2 = 4000 bytes |

## Formato de cada celda

```
+--------+--------+
| CHAR   | ATTR   |
+--------+--------+
byte 0   byte 1
```

### Byte 0 - Caracter

Codigo del caracter a mostrar (ASCII extendido, CP-437). Ejemplos usados en DemOS:

| Hex | Caracter | Uso |
|-----|----------|-----|
| `0x20` | (espacio) | Vacio/fondo |
| `0xDB` | `█` | Bloque relleno (pixeles de letras grandes) |
| `0xC9` | `╔` | Esquina superior izquierda de caja |
| `0xBB` | `╗` | Esquina superior derecha |
| `0xC8` | `╚` | Esquina inferior izquierda |
| `0xBC` | `╝` | Esquina inferior derecha |
| `0xCD` | `═` | Linea horizontal doble |
| `0xBA` | `║` | Linea vertical doble |

### Byte 1 - Atributo (color)

```
Bit:    7   6   5   4  |  3   2   1   0
       +-------+-------+-------+-------+
       |  BLINK   |  BG COLOR | FG COLOR |
       +-------+-------+-------+-------+
```

| Bits | Significado |
|------|-------------|
| 0-3 | Color de frente (foreground) |
| 4-6 | Color de fondo (background) |
| 7 | Parpadeo (blink) |

### Colores disponibles

| Codigo | Color | Codigo | Color |
|--------|-------|--------|-------|
| `BLACK` (0x0) | Negro | `DARKGREY` (0x8) | Gris oscuro |
| `BLUE` (0x1) | Azul | `LIGHTBLUE` (0x9) | Azul claro |
| `GREEN` (0x2) | Verde | `LIGHTGREEN` (0xA) | Verde claro |
| `CYAN` (0x3) | Cian | `LIGHTCYAN` (0xB) | Cian claro |
| `RED` (0x4) | Rojo | `LIGHTRED` (0xC) | Rojo claro |
| `MAGENTA` (0x5) | Magenta | `LIGHTMAGENTA` (0xD) | Magenta claro |
| `BROWN` (0x6) | Marron | `LIGHTBROWN` (0xE) | Marron claro |
| `LIGHTGREY` (0x7) | Gris claro | `WHITE` (0xF) | Blanco |

## vga.h - Definiciones y constantes

```c
#define FRAMEBUFFER 0x000B8000
#define CRTC_CMD_PORT   0x3D4
#define CRTC_DATA_PORT  0x3D5
```

### Puertos CRTC

| Puerto | Proposito |
|--------|-----------|
| `0x3D4` | Registro de comando (selecciona que registro CRTC modificar) |
| `0x3D5` | Registro de datos (lee/escribe el valor) |

### Registros CRTC usados

| Comando | Significado |
|---------|-------------|
| `0x0E` | Byte alto de la posicion del cursor |
| `0x0F` | Byte bajo de la posicion del cursor |
| `0x0C` | Byte alto del inicio de pantalla (scroll) |
| `0x0D` | Byte bajo del inicio de pantalla (scroll) |
| `0x0A` | Registro de inicio de cursor (estilo) |

## vga.c - Implementacion

### `write_letter_to_buffer()` — funcion base

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
1. Puntero `volatile` al framebuffer para evitar optimizaciones del compilador.
2. Atributo: `(fondo << 4) | frente`.
3. Combina: `(atributo << 8) | caracter`.
4. Posicion lineal: `fila * 80 + columna`.
5. Escribe en memoria de video.

### `write_letter_to_screen()` — atajo para fila 0, color blanco

```c
void write_letter_to_screen(const char c, uint16_t pos)
{
    write_letter_to_buffer(c, 0, pos, WHITE, BLACK);
}
```

Escribe un caracter en la fila 0 con color blanco sobre negro. Usada internamente por `print_byte()`.

### `write_to_screen()` — cadena completa

```c
void write_to_screen(const char *buf, uint16_t len)
{
    for (uint32_t i = 0; i < len; i++)
        write_letter_to_buffer(buf[i], 0, i, WHITE, BLACK);
    move_cursor(len);
}
```

### `move_cursor()` — posicion del cursor

```c
void move_cursor(uint16_t pos)
{
    uint16_t low = pos & 0x00FF;
    uint16_t high = (pos >> 8) & 0x00FF;
    outb(CRTC_CMD_PORT, CURSOR_POS_HIGH_BYTE_CMD);
    outb(CRTC_DATA_PORT, high);
    outb(CRTC_CMD_PORT, CURSOR_POS_LOW_BYTE_CMD);
    outb(CRTC_DATA_PORT, low);
}
```

Escribe los 2 bytes de la posicion en los registros CRTC (posicion 0-1999 para 80x25).

### `scroll()` — desplazamiento de pantalla

```c
void scroll(uint16_t row)
{
    uint16_t pos = 80 * row;
    // Escribe pos en SCREEN_START_POS (alto + bajo)
}
```

Desplaza la pantalla hacia arriba ajustando el registro de inicio de pantalla del CRTC.

### `print_byte()` — binario visual

```c
void print_byte(uint8_t *pbyte, uint32_t pos)
{
    for (int16_t bit = 0; bit < 8; bit++)
    {
        uint8_t mask = (uint8_t) 0x1 << (7 - bit);
        write_letter_to_screen(*pbyte & mask ? '1' : '0', pos + bit);
    }
}
```

Muestra los 8 bits de un byte como '1' y '0' en pantalla. Utiles para depuracion de registros de hardware.

### `style_cursor()` — control de cursor

```c
typedef enum { BIG, SMALL, DISABLE, ENABLE } CursorStyle;

void style_cursor(CursorStyle cstyle)
{
    switch (cstyle) {
    case BIG:     // cursor grueso (linea 0-15)
    case SMALL:   // cursor fino (linea 12-15)
    case DISABLE: // oculta el cursor
    case ENABLE:  // restaura el cursor
    }
}
```

Controla el registro `0x0A` (Cursor Start) del CRTC. Para desactivar, escribe `start | 0x20` (bit 5 = deshabilitar). Para habilitar, escribe `start & 0xBF` (bit 6 = habilitar).

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Librerias del sistema](librerias.md) | [Flujo de ejecucion](flujo-ejecucion.md) |

| Relacionados |
|--------------|
| [Kernel principal](kernel-principal.md) |
| [Librerias del sistema](librerias.md) |
