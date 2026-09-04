---
title: "Librerías del sistema"
order: 7
---

# Librerías del sistema

DemOS implementa sus propias librerías porque no puede usar la libc estándar (no hay SO anfitrión). Esta sección documenta los tipos base, operaciones de E/S y utilidades compartidas.

---

## 1. types.h — Definición de tipos enteros

**Archivo:** `include/types.h`

```c
#pragma once

#ifndef NULL
#define NULL 0
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
typedef int bool;
#define false 0
#define true 1
#endif

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

### ¿Por qué definir tipos personalizados?

Los tipos estándar (`int`, `long`) tienen tamaños dependientes de la plataforma. Para un SO necesitamos tamaños exactos:

| Tipo | Tamaño | Rango |
|------|--------|-------|
| `uint8_t` | 1 byte | 0 a 255 |
| `uint16_t` | 2 bytes | 0 a 65535 |
| `uint32_t` | 4 bytes | 0 a 2^32-1 |
| `uint64_t` | 8 bytes | 0 a 2^64-1 |
| `uintptr_t` | 4 bytes | Entero que almacena una dirección |

### `bool` condicional

DemOS define `bool`/`true`/`false` solo para estándares **anteriores a C23**, ya que en C23 son palabras clave del lenguaje:

```c
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
typedef int bool;
#define false 0
#define true 1
#endif
```

### `#pragma once`

Evita inclusión múltiple del archivo.

---

## 2. asm.h — Instrucciones de ensamblador inline

**Archivo:** `include/asm.h`

```c
#pragma once
#include "types.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}
```

### ¿Qué hacen?

| Función | Instrucción | Propósito |
|---------|-------------|-----------|
| `outb(port, value)` | `outb` | Envía 1 byte al puerto E/S |
| `inb(port)` | `inb` | Lee 1 byte desde el puerto E/S |
| `outl(port, value)` | `outl` | Envía 4 bytes (dword) al puerto E/S |
| `inl(port)` | `inl` | Lee 4 bytes (dword) desde el puerto E/S |

Se usan `outl`/`inl` para el acceso al espacio de configuración PCI (puertos `0xCF8`/`0xCFC`).

### Sintaxis de ensamblador inline en GCC

```
__asm__ volatile ( "instruccion" : salida : entrada );
```

| Elemento | Significado |
|----------|-------------|
| `__asm__` | GCC inline assembly keyword |
| `volatile` | Evita optimización/reordenamiento |
| `"a"(value)` | Coloca `value` en AX |
| `"Nd"(port)` | Coloca `port` en DX |
| `"=a"(ret)` | Toma AX como resultado |

---

## 3. util_lib.h / util_lib.c — strlen

**Archivo:** `include/util/util_lib.h` y `src/util/util_lib.c`

```c
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

## 4. timer.h / timer.c — delay()

**Archivo:** `include/drivers/timer.h` y `src/drivers/timer.c`

```c
void delay(uint32_t iterations)
{
    for (volatile uint32_t i = 0; i < iterations; i++);
}
```

### Funcionamiento

Bucle de espera activa (busy-wait). **`volatile`** evita que el compilador optimice el bucle y lo elimine.

No hay temporizadores ni interrupciones de timer configurados como drivers, por lo que esta es una forma simple de crear pausas para la animación del splash.

| Uso | Iteraciones | Efecto aprox. (QEMU) |
|-----|-------------|----------------------|
| Transición rápida | 15,000,000 | ~0.15s |
| Pausa entre letras | 30,000,000 | ~0.3s |
| Pulso colectivo | 40,000,000 | ~0.4s |
| Pausa larga | 60,000,000 | ~0.6s |

---

## 5. big_text.h / big_text.c — Letras grandes y cajas

**Archivo:** `include/util/big_text.h` y `src/util/big_text.c`

Proporciona glyphs (mapas de bits) para letras de 5x5, dibujo de cajas y renderizado de caracteres grandes. Se usa en la animación de arranque (`animate_splash()`).

### Constantes

```c
#define CHAR_W 5
#define CHAR_H 5
```

### Glyphs disponibles

```c
extern const uint8_t glyph_D[CHAR_H][CHAR_W];
extern const uint8_t glyph_e[CHAR_H][CHAR_W];
extern const uint8_t glyph_m[CHAR_H][CHAR_W];
extern const uint8_t glyph_O[CHAR_H][CHAR_W];
extern const uint8_t glyph_S[CHAR_H][CHAR_W];
```

Por ejemplo, `glyph_D`:

```c
const uint8_t glyph_D[CHAR_H][CHAR_W] = {
    {1, 1, 1, 1, 0},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 0},
};
```

Que representa visualmente:

```
████
█   █
█   █
█   █
████
```

### Funciones

#### `draw_box()`

```c
void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
```

Dibuja una caja con bordes dobles usando los caracteres de CP-437:

| Carácter | Valor | Descripción |
|----------|-------|-------------|
| `╔` | `0xC9` | Esquina superior izquierda |
| `╗` | `0xBB` | Esquina superior derecha |
| `╚` | `0xC8` | Esquina inferior izquierda |
| `╝` | `0xBC` | Esquina inferior derecha |
| `═` | `0xCD` | Línea horizontal |
| `║` | `0xBA` | Línea vertical |

#### `draw_big_char()`

```c
void draw_big_char(const uint8_t (*glyph)[CHAR_W], uint16_t r0, uint16_t c0, uint8_t color);
```

Renderiza un glyph de 5x5 en la posición `(r0, c0)`. Usa `0xDB` (`█`) para los píxeles llenos y espacio para los vacíos.
