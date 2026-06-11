/* types.h - Definiciones de tipos enteros para el kernel en modo protegido.
   Proporciona tipos de ancho fijo sin depender de la libc. */

#pragma once

#define NULL 0

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint32_t uintptr_t;
