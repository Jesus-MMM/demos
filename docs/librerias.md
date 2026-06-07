# Librerias del sistema

DemOS implementa sus propias librerias porque no puede usar la libc estandar (no hay sistema operativo anfitrion).

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

Los tipos estandar (`int`, `long`, etc.) tienen tamanos **dependientes de la plataforma** (en unas son 16 bits, en otras 32). Para un SO necesitamos tipos con tamanos **exactos y predecibles**:

| Tipo | Tamano | Rango |
|------|--------|-------|
| `uint8_t` | 1 byte | 0 a 255 |
| `uint16_t` | 2 bytes | 0 a 65535 |
| `uint32_t` | 4 bytes | 0 a 4,294,967,295 |
| `uint64_t` | 8 bytes | 0 a 2^64-1 |
| `int8_t` | 1 byte | -128 a 127 |
| `int16_t` | 2 bytes | -32,768 a 32,767 |
| `int32_t` | 4 bytes | -2,147,483,648 a 2,147,483,647 |
| `int64_t` | 8 bytes | -2^63 a 2^63-1 |
| `uintptr_t` | 4 bytes (32 bits) | Entero capaz de almacenar una direccion de memoria |

### `#pragma once`

Directiva que evita que el archivo se incluya mas de una vez (equivale a un `#ifndef` guard).

---

## 2. asm.h - Instrucciones de ensamblador inline

**Archivo:** `include/asm.h`

```c
#pragma once
#include "types.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile(
        "inb %1, %0"
        : "=a" (ret)
        : "Nd" (port)
    );
    return ret;
}
```

### ¿Que hacen estas funciones?

Permiten comunicarse directamente con los puertos de E/S del hardware x86:

| Funcion | Instruccion | Proposito |
|---------|-------------|-----------|
| `outb(port, value)` | `outb` | Envia 1 byte al puerto especificado |
| `inb(port)` | `inb` | Lee 1 byte desde el puerto especificado |

### Sintaxis de ensamblador inline en GCC

```
__asm__ volatile ( "instruccion" : salida : entrada );
```

| Elemento | Significado |
|----------|-------------|
| `__asm__` | Palabra clave de GCC para ensamblador inline |
| `volatile` | Evita que el compilador optimice/reordene la instruccion |
| `"a"(value)` | Coloca `value` en el registro `AX` (o `EAX`) |
| `"Nd"(port)` | Coloca `port` en el registro `DX` |
| `"=a"(ret)` | Toma el resultado del registro `AX` y lo asigna a `ret` |

### `static inline`

- **`static`**: La funcion solo es visible en el archivo donde se incluye.
- **`inline`**: El compilador reemplaza la llamada con el codigo directamente (sin llamada de funcion), importante para operaciones de bajo nivel.

---

## 3. util_lib.h / util_lib.c - strlen

**Archivo:** `include/util_lib.h` y `lib/util_lib.c`

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
    if (str == NULL)
        return -1;

    const char *start = str;
    while (*str != '\0')
        str++;

    return (int64_t)(str - start);
}
```

### Funcionamiento

1. Guarda el puntero inicial.
2. Avanza caracter por caracter hasta encontrar `'\0'` (null terminator).
3. Resta la direccion final menos la inicial para obtener la longitud.

### Manejo de errores

- Si `str` es `NULL`, retorna `-1` para indicar error (la libc estandar devuelve `0` o crashea).

### ¿Por que `int64_t` en lugar de `size_t`?

En sistemas estandar, `strlen` devuelve `size_t` (unsigned). DemOS usa `int64_t` para poder retornar `-1` en caso de error, ya que no tenemos `size_t` definido.

---

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Kernel principal](kernel-principal.md) | [VGA y Framebuffer](vga-framebuffer.md) |

| Relacionados |
|--------------|
| [VGA y Framebuffer](vga-framebuffer.md) |
