---
title: "Entorno y herramientas"
order: 2
---

# Entorno de desarrollo y herramientas

## Requisitos

| Herramienta | Propósito | Instalación (Ubuntu/Debian) |
|-------------|-----------|----------------------------|
| **GCC** | Compilador cruzado para i386 | `apt install gcc gcc-multilib` |
| **NASM** | Ensamblador para el loader | `apt install nasm` |
| **LD** | Enlazador (linker) | `apt install binutils` |
| **GRUB** | Crear ISO booteable | `apt install grub-pc-bin` |
| **xorriso** | Generar ISO | `apt install xorriso` |
| **QEMU** | Emulador para pruebas | `apt install qemu-system-x86` |
| **mtools** | Crear discos FAT32 | `apt install mtools` |
| **mkfs.fat** | Formatear disco virtual | `apt install dosfstools` |

## Compilación cruzada

Aunque GCC compile en modo 32 bits con la bandera `-m32`, para un SO real se recomienda un **compilador cruzado** (i686-elf-gcc) para evitar dependencias del sistema anfitrión. DemOS usa banderas especiales para mantenerlo autónomo:

```
-nostdlib -nostdinc -fno-builtin -fno-stack-protector
-nostartfiles -nodefaultlibs -ffreestanding
-mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
```

> **`-ffreestanding`**: Indica que el código se ejecutará sin soporte del sistema operativo (sin libc, sin startup files).

> **`-mno-sse` y similares**: Desactivan instrucciones SIMD que podrían no estar disponibles en hardware real.

## Compilar y ejecutar

```bash
make            # Compila todo y genera DemOS.iso + demos.img
make runqemu    # Compila y ejecuta en QEMU con disco FAT32
make clean      # Limpia archivos generados
```

## Verificar la compilación

```bash
file build/kernel.elf
# Debe mostrar: ELF 32-bit MSB executable, Intel 80386
```

## Disco virtual FAT32

DemOS genera un disco virtual `demos.img` (32 MB) que se adjunta a QEMU y contiene:

| Archivo | Propósito |
|---------|-----------|
| `/HELLO.TXT` | Archivo de texto de prueba |
| `/FONDO.PNG` | Imagen de fondo del escritorio |
| `/docs/README.TXT` | Documentación en disco |

El disco se crea automáticamente durante `make` usando `dd`, `mkfs.fat`, `mmd` y `mcopy`.
