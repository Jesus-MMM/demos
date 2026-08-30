/* types.h - Definiciones de tipos enteros para el kernel en modo protegido.
   Proporciona tipos de ancho fijo sin depender de la libc. */

#pragma once

#define NULL 0

/* En C23 (y posteriores), bool, true y false ya son palabras clave del
   lenguaje; solo se definen para estandares anteriores. */
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
