# DemOS

Sistema operativo minimalista desarrollado desde cero con fines educativos.

## Requisitos rapidos

- GCC (i386), NASM, LD, GRUB, xorriso, QEMU

```bash
make
make runqemu
```

## Documentacion

| # | Documento | Descripcion |
|---|-----------|-------------|
| 1 | [Introduccion](docs/introduccion.md) | Que es DemOS y estructura del proyecto |
| 2 | [Arquitectura](docs/arquitectura.md) | Capas del sistema y flujo de datos |
| 3 | [Entorno y setup](docs/entorno-setup.md) | Herramientas y comandos |
| 4 | [Makefile](docs/makefile.md) | Sistema de compilacion |
| 5 | [Linker script](docs/linker-script.md) | Script de enlace y mapa de memoria |
| 6 | [Loader (ensamblador)](docs/loader-ensamblador.md) | Punto de entrada en NASM |
| 7 | [Kernel principal](docs/kernel-principal.md) | Entry point en C |
| 8 | [Librerias](docs/librerias.md) | Tipos, E/S, timer, big_text, splash |
| 9 | [VGA Framebuffer](docs/vga-framebuffer.md) | Escritura en pantalla |
| 10 | [Flujo de ejecucion](docs/flujo-ejecucion.md) | Recorrido completo del codigo |
