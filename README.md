# DemOS

Sistema operativo minimalista desarrollado desde cero con fines educativos.

## Requisitos rapidos

- GCC (i386), NASM, LD, GRUB, xorriso, QEMU

```bash
make
make runqemu
```

## Documentacion

La documentacion esta disponible en el sitio web generado con [md2site](https://github.com/Jesus-MMM/markdown-to-website): [https://Jesus-MMM.github.io/demos/](https://Jesus-MMM.github.io/demos/)

Tambien puedes consultarla directamente desde el repositorio en [docs/](docs/):

| Seccion | Contenido |
|---------|-----------|
| [Introduccion](docs/introduccion/index.md) | Que es DemOS, entorno, arquitectura y flujo de ejecucion |
| [Kernel](docs/kernel/index.md) | Loader, linker script, kernel principal, GDT, interrupciones y librerias |
| [Drivers](docs/drivers/index.md) | VGA, teclado, mouse, puerto serie y controlador PCI |
| [Build](docs/build/index.md) | Sistema de compilacion con Makefile |
