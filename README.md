# DemOS

Sistema operativo minimalista desarrollado desde cero con fines educativos.

## Requisitos rapidos

- GCC, NASM, LD, GRUB, xorriso, QEMU

```bash
# Compilar y ejecutar
make
make runqemu
```

## Documentacion

La documentacion completa esta disenada para personas sin experiencia en desarrollo de sistemas operativos.

| # | Documento | Descripcion |
|---|-----------|-------------|
| 1 | [Introduccion](docs/introduccion.md) | Que es DemOS y estructura del proyecto |
| 2 | [Arquitectura](docs/arquitectura.md) | Capas del sistema y flujo de datos |
| 3 | [Entorno y setup](docs/entorno-setup.md) | Herramientas necesarias y comandos |
| 4 | [Makefile](docs/makefile.md) | Sistema de compilacion explicado |
| 5 | [Linker script](docs/linker-script.md) | Script de enlace y mapa de memoria |
| 6 | [Loader (ensamblador)](docs/loader-ensamblador.md) | Punto de entrada del kernel en NASM |
| 7 | [Kernel principal](docs/kernel-principal.md) | Corazon del SO en C |
| 8 | [Librerias](docs/librerias.md) | Tipos, puertos E/S, utilidades |
| 9 | [VGA Framebuffer](docs/vga-framebuffer.md) | Como se escribe en pantalla |
| 10 | [Flujo de ejecucion](docs/flujo-ejecucion.md) | Recorrido completo del codigo |
