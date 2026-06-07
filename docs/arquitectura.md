# Arquitectura general de DemOS

## Capas del sistema

```
+---------------------------+
|       GRUB (bootloader)   |  <- Carga el kernel en memoria
+---------------------------+
|   loader.s (ensamblador)  |  <- Configura el stack y llama a C
+---------------------------+
|    kernelmain.c (kernel)  |  <- Punto de entrada en C
+---------------------------+
|      Librerias (lib/)     |  <- Funciones de soporte (io, util)
+---------------------------+
|      Hardware (VGA)       |  <- Pantalla, puertos E/S
+---------------------------+
```

## Modo de ejecucion

DemOS opera en **32 bits (protegido)** desde el inicio. GRUB se encarga de cambiar el CPU a modo protegido antes de transferir el control al kernel.

## Espacio de direccionamiento

- El kernel se carga en la direccion `0x00100000` (1 MB), como exige la especificacion Multiboot.
- La pila del kernel se reserva en el BSS con 4 KB de tamaño.

## Flujo de datos

1. **GRUB** lee `grub.cfg` y carga `kernel.elf` en memoria.
2. **loader.s** recibe el control, configura el stack y llama a `kernel_main()`.
3. **kernel_main()** usa las funciones de `io.c` para escribir en el framebuffer VGA.
4. El texto se renderiza directamente en la memoria de video en `0xB8000`.

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Introduccion](introduccion.md) | [Entorno y herramientas](entorno-setup.md) |

| Relacionados |
|--------------|
| [Loader en ensamblador](loader-ensamblador.md) |
| [Kernel principal](kernel-principal.md) |
| [Librerias del sistema](librerias.md) |
