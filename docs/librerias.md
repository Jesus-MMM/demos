# Librerias del sistema

DemOS implementa sus propias librerias porque no puede usar la libc estandar (no hay SO anfitrion).

---

## 1. types.h - Definicion de tipos enteros

**Archivo:** `include/types.h`

```c
#pragma once

#define NULL 0

typedef signed char             int8_t;
typedef short                   int16_t;
typedef int                     int32_t;
typedef long long               int64_t;

typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long      uint64_t;

typedef uint32_t                uintptr_t;
```

### ¿Por que definir tipos personalizados?

Los tipos estandar (`int`, `long`) tienen tamanos dependientes de la plataforma. Para un SO necesitamos tamanos exactos:

| Tipo | Tamaño | Rango |
|------|--------|-------|
| `uint8_t` | 1 byte | 0 a 255 |
| `uint16_t` | 2 bytes | 0 a 65535 |
| `uint32_t` | 4 bytes | 0 a 2^32-1 |
| `uint64_t` | 8 bytes | 0 a 2^64-1 |
| `uintptr_t` | 4 bytes | Entero que almacena una direccion |

### `#pragma once`

Evita inclusion multiple del archivo.

---

## 2. asm.h - Instrucciones de ensamblador inline

**Archivo:** `include/asm.h`

```c
#pragma once
#include "types.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1" : : "a"(value), "Nd"(port)
    );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile(
        "inb %1, %0" : "=a" (ret) : "Nd" (port)
    );
    return ret;
}
```

### ¿Que hacen?

| Funcion | Instruccion | Proposito |
|---------|-------------|-----------|
| `outb(port, value)` | `outb` | Envia 1 byte al puerto E/S |
| `inb(port)` | `inb` | Lee 1 byte desde el puerto E/S |

### Sintaxis de ensamblador inline en GCC

```
__asm__ volatile ( "instruccion" : salida : entrada );
```

| Elemento | Significado |
|----------|-------------|
| `__asm__` | GCC inline assembly keyword |
| `volatile` | Evita optimizacion/reordenamiento |
| `"a"(value)` | Coloca `value` en AX |
| `"Nd"(port)` | Coloca `port` en DX |
| `"=a"(ret)` | Toma AX como resultado |

---

## 3. util_lib.h / util_lib.c - strlen

**Archivo:** `include/util/util_lib.h` y `src/util/util_lib.c`

```c
// util_lib.h
#pragma once
#include "types.h"
int64_t strlen(const char *str);
```

```c
// util_lib.c
#include "util_lib.h"

int64_t strlen(const char *str)
{
    if (str == NULL) return -1;
    const char *start = str;
    while (*str != '\0') str++;
    return (int64_t)(str - start);
}
```

Recorre la cadena hasta `'\0'` y retorna la diferencia de punteros. Retorna `-1` si la entrada es NULL.

---

## 4. timer.h / timer.c - delay()

**Archivo:** `include/drivers/timer.h` y `src/drivers/timer.c`

```c
// timer.h
#pragma once
#include "types.h"
void delay(uint32_t iterations);
```

```c
// timer.c
#include "timer.h"

void delay(uint32_t iterations)
{
    for (volatile uint32_t i = 0; i < iterations; i++);
}
```

### Funcionamiento

Bucle de espera activa (busy-wait). **`volatile`** evita que el compilador optimice el bucle y lo elimine.

No hay temporizadores ni interrupciones configuradas en DemOS, por lo que esta es la unica forma de crear pausas. Los valores tipicos:

| Uso | Iteraciones | Efecto aprox. (QEMU) |
|-----|-------------|----------------------|
| Transicion rapida | 15,000,000 | ~0.15s |
| Pausa entre letras | 30,000,000 | ~0.3s |
| Pulso colectivo | 40,000,000 | ~0.4s |
| Pausa larga | 60,000,000 | ~0.6s |

---

## 5. big_text.h / big_text.c - Letras grandes y cajas

**Archivo:** `include/util/big_text.h` y `src/util/big_text.c`

Proporciona glyphs (mapas de bits) para letras de 5x5, dibujo de cajas y renderizado de caracteres grandes.

### Constantes

```c
#define CHAR_W 5
#define CHAR_H 5
```

Cada letra se define como una matriz de 5x5 donde `1` = bloque lleno, `0` = espacio:

```
Glyph 'D' (5x5):      Glyph 'e' (5x5):
████                  █████
█   █                █
█   █                █████
█   █                █
████                  █████
```

### Funciones

#### `draw_box()`

```c
void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
```

Dibuja una caja con bordes dobles usando los caracteres de CP-437:

- `0xC9` (`╔`) = esquina superior izquierda
- `0xBB` (`╗`) = esquina superior derecha
- `0xC8` (`╚`) = esquina inferior izquierda
- `0xBC` (`╝`) = esquina inferior derecha
- `0xCD` (`═`) = linea horizontal
- `0xBA` (`║`) = linea vertical

#### `draw_big_char()`

```c
void draw_big_char(const uint8_t (*glyph)[CHAR_W], uint16_t r0, uint16_t c0, uint8_t color);
```

Renderiza un glyph de 5x5 en la posicion `(r0, c0)`. Usa `0xDB` (`█`) para los pixeles llenos y espacio para los vacios.

### Glyphs disponibles

```c
extern const uint8_t glyph_D[CHAR_H][CHAR_W];
extern const uint8_t glyph_e[CHAR_H][CHAR_W];
extern const uint8_t glyph_m[CHAR_H][CHAR_W];
extern const uint8_t glyph_O[CHAR_H][CHAR_W];
extern const uint8_t glyph_S[CHAR_H][CHAR_W];
```

---

## 6. splash.h / splash.c - Animacion de bienvenida

**Archivo:** `include/util/splash.h` y `src/util/splash.c`

### Declaracion

```c
void animate_splash(void);
```

### Macros de layout

Calculan las posiciones centradas para la caja y el texto:

```c
#define N_LETTERS 5                    // "DemOS"
#define TXT_W (5*5 + 4) = 29          // Ancho del texto
#define TXT_H 5                       // Alto del texto
#define PAD 3                         // Padding interno de la caja
#define B_W 37                        // Ancho total de la caja
#define B_H 9                         // Alto total de la caja
#define B_C 21                        // Columna inicial de la caja (centrada)
#define B_R 8                         // Fila inicial de la caja (centrada)
#define T_C 25                        // Columna inicial del texto
#define T_R 10                        // Fila inicial del texto
```

### Algoritmo de animacion

```
1. draw_box(B_C, B_R, B_W, B_H, GREEN)     → caja centrada
2. Para cada letra i en "DemOS":
     a. draw_big_char(glyph[i], DARKGREY)   → fantasma gris
     b. delay(30000000)
     c. draw_big_char(glyph[i], GREEN)      → verde medio
     d. delay(15000000)
     e. draw_big_char(glyph[i], LIGHTGREEN) → verde brillante
     f. delay(15000000)
3. delay(60000000)                           → pausa
4. 2 veces:
     a. Todas las letras en GREEN
     b. delay(40000000)
     c. Todas en LIGHTGREEN
     d. delay(40000000)
5. Todas en LIGHTGREEN (estado final)
```

### Dependencias

`splash.c` incluye y usa:

| Modulo | Funcion usada |
|--------|---------------|
| `big_text.h` | `draw_box()`, `draw_big_char()`, glyphs |
| `timer.h` | `delay()` |
| `vga.h` | Constantes de color (`GREEN`, `LIGHTGREEN`, `DARKGREY`, `BLACK`) |

---

## 7. mouse.h / mouse.c - Driver de mouse PS/2

**Archivo:** `include/drivers/mouse.h` y `src/drivers/mouse.c`

### Declaracion

```c
void mouse_init(void);
uint32_t mouse_handler(uint32_t esp);
void mouse_set_cursor(uint16_t row, uint16_t col);
```

### Funcionamiento

El driver de mouse maneja IRQ 12 (interrupcion `0x2C`) y procesa paquetes de 3 bytes del mouse PS/2:

| Byte | Contenido |
|------|-----------|
| 0 | Botones (bit 0=izq, bit 1=der, bit 2=central) + flags |
| 1 | Movimiento X (offset desde ultimo paquete) |
| 2 | Movimiento Y (offset desde ultimo paquete) |

Actualmente implementa un **cursor visual** que invierte los colores VGA en la posicion del mouse.

### Dependencias

| Modulo | Funcion usada |
|--------|---------------|
| `asm.h` | `inb()`, `outb()` |
| `vga.h` | Acceso directo a framebuffer VGA (`0xB8000`) |
| `serial.h` | `serial_write_string()` (mensaje de activacion) |

---

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Kernel principal](kernel-principal.md) | [VGA y Framebuffer](vga-framebuffer.md) |

| Relacionados |
|--------------|
| [VGA y Framebuffer](vga-framebuffer.md) |
