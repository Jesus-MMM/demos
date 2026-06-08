# Kernel principal (kernelmain.c)

## Proposito

`kernelmain.c` es el punto de entrada del kernel en C. Sirve como orquestador minimo que delega toda la logica visual a los modulos especializados.

## Contenido completo

```c
#include "io.h"
#include "splash.h"

int kernel_main()
{
    style_cursor(DISABLE);
    animate_splash();
    return 0;
}
```

## Explicacion

### Includes

| Include | Proposito |
|---------|-----------|
| `#include "io.h"` | `style_cursor()` para desactivar el cursor parpadeante |
| `#include "splash.h"` | `animate_splash()` para la animacion de bienvenida |

### `style_cursor(DISABLE)`

Desactiva el cursor de texto parpadeante del hardware VGA escribiendo en el puerto CRTC `0x3D4`/`0x3D5`. Esto evita que el cursor distraiga durante la animacion.

### `animate_splash()`

Funcion definida en `lib/splash.c` que orquesta toda la animacion:

1. Dibuja una caja centrada con bordes dobles (`draw_box()`).
2. Anima cada letra de "DemOS" en grande (5x5): gris fantasma → verde → verde brillante.
3. Hace pulsar todas las letras juntas dos veces.
4. Deja el texto fijo en verde brillante.

### Funcion `kernel_main()`

Retorna `int` (aunque nunca se usa, el loader queda en bucle infinito). En un SO real recibiria el magic number de GRUB y un puntero a informacion del hardware.

## Modularidad

El kernel principal se mantiene propositivamente minimalista. Toda la logica compleja vive en modulos separados:

| Modulo | Responsabilidad |
|--------|-----------------|
| `splash.c` | Animacion y layout centrado |
| `big_text.c` | Letras grandes 5x5, dibujo de cajas |
| `timer.c` | Espera activa (delay) |
| `io.c` | Escritura directa en framebuffer VGA |

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Loader en ensamblador](loader-ensamblador.md) | [Librerias del sistema](librerias.md) |

| Relacionados |
|--------------|
| [VGA y Framebuffer](vga-framebuffer.md) |
| [Flujo de ejecucion](flujo-ejecucion.md) |
