---
title: "Tipos y E/S"
order: 5
---

# Tipos enteros y puertos de E/S (`include/types.h` / `include/asm.h`)

## `types.h` — Tipos enteros de ancho fijo

Como DemOS es un kernel **freestanding** (sin libc), define sus propios tipos de ancho fijo:

```c
#ifndef NULL
#define NULL 0
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
typedef int bool;
#define false 0
#define true 1
#endif

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint32_t uintptr_t;
```

| Tipo | Tamaño | Descripción |
|------|--------|-------------|
| `int8_t` / `uint8_t` | 8 bits | Enteros con/sin signo |
| `int16_t` / `uint16_t` | 16 bits | Enteros con/sin signo |
| `int32_t` / `uint32_t` | 32 bits | Enteros con/sin signo |
| `int64_t` / `uint64_t` | 64 bits | Enteros con/sin signo |
| `uintptr_t` | 32 bits | Puntero como entero sin signo |

> **Nota**: en C23 (y posteriores), `bool`, `true` y `false` ya son palabras clave del lenguaje; solo se definen para estándares anteriores (se comprueba `__STDC_VERSION__ < 202311L`).

## `asm.h` — E/S sobre puertos con ensamblador inline

`asm.h` proporciona acceso a los **puertos de E/S** del procesador x86 mediante instrucciones ensamblador inline.

### API

| Función | Instrucción | Descripción |
|---------|-------------|-------------|
| `outb(port, value)` | `outb` | Escribe un byte en un puerto |
| `inb(port)` | `inb` | Lee un byte de un puerto |
| `outl(port, value)` | `outl` | Escribe un dword (32 bits) en un puerto |
| `inl(port)` | `inl` | Lee un dword (32 bits) de un puerto |

### Implementación de `outb`

```c
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
```

El ensamblador inline usa la convención de registro: `"a"` (eax/al) para el valor y `"Nd"` (edx/dx) para el puerto.

### Implementación de `inb`

```c
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
```

## Uso típico

| Módulo | Puertos usados vía asm.h |
|--------|--------------------------|
| `drivers/serial.h` | `0x3F8` (COM1) — `inb`/`outb` |
| `drivers/keyboard.h`/`mouse.h` | `0x60`/`0x64` — `inb`/`outb` |
| `kernel/pci.c` | `0xCF8`/`0xCFC` — `inl`/`outl` |
| `drivers/vga.h` | Registros de VGA — `inb`/`outb` |

## Dependencias

| Módulo | Uso |
|--------|-----|
| `types.h` | Tipos enteros usados por las funciones |
