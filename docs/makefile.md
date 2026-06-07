# Sistema de compilacion (Makefile)

## Vision general

El `Makefile` automatiza todo el proceso de compilacion, enlazado y generacion de la ISO.

```makefile
INCDIRS = ./include/
CODEDIRS = ./ ./lib/

CC = gcc
CFLAGS = -m32 -Wall -Wextra -Werror -g \
         $(foreach D, $(INCDIRS), -I$(D)) \
         -MP -MD \
         -nostdlib -nostdinc -fno-builtin \
         -fno-stack-protector -nostartfiles \
         -nodefaultlibs -ffreestanding

AS = nasm
ASFLAGS = -f elf32
LDFLAGS = -T link.ld -melf_i386
```

## Desglose de banderas

### Compilacion (`CFLAGS`)

| Bandera | Explicacion |
|---------|-------------|
| `-m32` | Genera codigo de 32 bits |
| `-Wall -Wextra -Werror` | Activa todos los warnings y los trata como errores |
| `-g` | Incluye informacion de depuracion (debug symbols) |
| `-nostdlib` | No vincula la biblioteca estandar de C |
| `-nostdinc` | No busca cabeceras en directorios del sistema |
| `-fno-builtin` | Desactiva optimizaciones de funciones built-in de GCC |
| `-fno-stack-protector` | Desactiva la proteccion de pila (stack canary) |
| `-nostartfiles` | No usa archivos de inicio estandar (crt0) |
| `-nodefaultlibs` | No vincula bibliotecas por defecto |
| `-ffreestanding` | Modo autonomo (sin SO anfitrion) |
| `-MP -MD` | Genera dependencias automaticas (archivos .d) |

### Ensamblador (`ASFLAGS`)

| Bandera | Explicacion |
|---------|-------------|
| `-f elf32` | Genera un archivo objeto en formato ELF de 32 bits |

### Enlazador (`LDFLAGS`)

| Bandera | Explicacion |
|---------|-------------|
| `-T link.ld` | Usa un script de enlace personalizado |
| `-melf_i386` | Emula el enlazador de i386 (ELF 32 bits) |

## Objetivos (targets)

| Comando | Que hace |
|---------|----------|
| `make` / `make all` | Compila todo y genera `DemOS.iso` |
| `make runqemu` | Compila y ejecuta en QEMU |
| `make clean` | Elimina `build/`, `iso/` y `DemOS.iso` |
| `make cleanrunqemu` | Limpia, recompila y ejecuta |

## Generacion de la ISO

```makefile
all: $(BUILDDIR)/kernel.elf
    mkdir -p iso/boot/grub
    cp grub.cfg iso/boot/grub/grub.cfg
    cp $(BUILDDIR)/kernel.elf iso/boot/kernel.elf
    grub-mkrescue -o DemOS.iso iso -d /usr/lib/grub/i386-pc
```

Pasos:
1. Copia `grub.cfg` y `kernel.elf` a `iso/boot/`
2. `grub-mkrescue` genera una ISO booteable compatible con la especificacion de GRUB

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Entorno y herramientas](entorno-setup.md) | [Loader en ensamblador](loader-ensamblador.md) |

| Relacionados |
|--------------|
| [Script de enlace (link.ld)](linker-script.md) |
