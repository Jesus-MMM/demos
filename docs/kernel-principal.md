# Kernel principal (kernelmain.c)

## Proposito

`kernelmain.c` es el nucleo del sistema operativo, escrito en C. Es el primer codigo de alto nivel que se ejecuta.

## Contenido completo

```c
#include "io.h"
#include "types.h"
#include "util_lib.h"

int kernel_main()
{
    const char *text = "Welcome to DemOS";
    write_to_screen(text, strlen(text));
    return 0;
}
```

## Explicacion linea por linea

### Includes

| Include | Proposito |
|---------|-----------|
| `#include "io.h"` | Funciones de escritura en pantalla (`write_to_screen`) |
| `#include "types.h"` | Definiciones de tipos (`char`, `int`, `uint16_t`, etc.) |
| `#include "util_lib.h"` | Utilidades como `strlen` |

> **Nota**: Se usan comillas `""` en lugar de `<>` porque las cabeceras estan en el directorio local `include/`, no en el sistema.

### Funcion `kernel_main()`

```c
int kernel_main()
```

La funcion retorna `int` (aunque el valor nunca se usa porque el loader entra en un bucle infinito despues). No recibe parametros.

En un SO real, esta funcion recibiria:
- **Magic number** de GRUB (`0x2BADB002`) para verificar que fue cargado por GRUB.
- **Puntero a la estructura de informacion** de GRUB (mapa de memoria, framebuffer, etc.).

### Cadena de texto

```c
const char *text = "Welcome to DemOS";
```

La palabra clave `const` indica que la cadena es inmutable. El compilador la coloca en la seccion `.rodata` (solo lectura).

### Escritura en pantalla

```c
write_to_screen(text, strlen(text));
```

1. `strlen(text)` calcula cuantos caracteres tiene la cadena (14 incluyendo el espacio).
2. `write_to_screen()` escribe cada caracter en el framebuffer VGA en la fila 0, con color blanco sobre fondo negro.

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Loader en ensamblador](loader-ensamblador.md) | [Librerias del sistema](librerias.md) |

| Relacionados |
|--------------|
| [VGA y Framebuffer](vga-framebuffer.md) |
| [Flujo de ejecucion](flujo-ejecucion.md) |
