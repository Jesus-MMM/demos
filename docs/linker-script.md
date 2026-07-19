# Script de enlace (link.ld)

## Que es un linker script?

Un **linker script** le indica al enlazador (`ld`) como organizar las diferentes secciones del kernel en la memoria. Es esencial en desarrollo de SO porque no usamos el layout estandar de un ejecutable normal.

## Contenido completo

```ld
ENTRY(loader)

SECTIONS {
    . = 0x00100000;

    .multiboot ALIGN(4) : { *(.multiboot) }
    .text      ALIGN(0x1000) : { *(.text) }
    .rodata    ALIGN(0x1000) : { *(.rodata) }
    .data      ALIGN(0x1000) : { *(.data) }
    .bss       ALIGN(0x1000) : { *(COMMON) *(.bss) }
}
```

## Explicacion seccion por seccion

### `ENTRY(loader)`

Define el **punto de entrada** del kernel: la etiqueta `loader` definida en `asm/loader.s`. Sin esto, el enlazador usaria `_start` por defecto.

### `. = 0x00100000;`

Establece la **direccion base** del kernel en `0x00100000` (1 MB). Esta es la direccion convencional donde GRUB carga los kernels Multiboot.

### `.multiboot ALIGN(4)`

Contiene la **cabecera Multiboot** (definida en `asm/loader.s`). Debe estar alineada a 4 bytes y aparecer en los primeros 8 KB del kernel para que GRUB la reconozca.

### `.text ALIGN(0x1000)`

Contiene el **codigo ejecutable** del kernel (instrucciones de maquina). Alineado a 4 KB (tamano de pagina).

### `.rodata`

Contiene datos de **solo lectura** (como cadenas de texto: `"Welcome to DemOS"`).

### `.data`

Contiene **variables globales inicializadas**.

### `.bss`

Contiene **variables globales no inicializadas** o inicializadas a cero, como la pila del kernel (`kernel_stack`). Se alinea a 4 KB.

> **Nota**: En el BSS, `*(COMMON)` recoge los simbolos comunes (generados por compiladores viejos) y `*(.bss)` recoge las variables sin inicializar.

## Mapa de memoria resultante

```
Direccion    Seccion        Contenido
0x00100000   .multiboot     Cabecera Multiboot (16 bytes)
0x00100004   .text          Codigo del loader + kernel
0x00101000   .rodata        "Welcome to DemOS"
0x00102000   .data          (variables inicializadas)
0x00103000   .bss           Pila del kernel (4 KB)
```

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Sistema de compilacion](makefile.md) | [Loader en ensamblador](loader-ensamblador.md) |

| Relacionados |
|--------------|
| [Makefile](makefile.md) |
