---
title: "Entorno y herramientas"
order: 2
---

# Entorno de desarrollo y herramientas

## Requisitos

| Herramienta | Proposito | Instalacion (Ubuntu/Debian) |
|-------------|-----------|-----------------------------|
| **GCC** | Compilador cruzado para i386 | `apt install gcc gcc-multilib` |
| **NASM** | Ensamblador para el loader | `apt install nasm` |
| **LD** | Enlazador (linker) | `apt install binutils` |
| **GRUB** | Crear ISO booteable | `apt install grub-pc-bin` |
| **xorriso** | Generar ISO | `apt install xorriso` |
| **QEMU** | Emulador para pruebas | `apt install qemu-system-x86` |

## Compilacion cruzada

Aunque GCC compile en modo 32 bits con la bandera `-m32`, para un SO real se recomienda un **compilador cruzado** (i686-elf-gcc) para evitar dependencias del sistema anfitrion. DemOS usa banderas especiales para mantenerlo autonoma:

```
-nostdlib -nostdinc -fno-builtin -fno-stack-protector
-nostartfiles -nodefaultlibs -ffreestanding
```

> **`-ffreestanding`**: Indica que el codigo se ejecutara sin soporte del sistema operativo (sin libc, sin startup files).

## Compilar y ejecutar

```bash
make            # Compila todo y genera DemOS.iso
make runqemu    # Compila y ejecuta en QEMU
make clean      # Limpia archivos generados
```

## Verificar la compilacion

```bash
file build/kernel.elf
# Debe mostrar: ELF 32-bit MSB executable, Intel 80386
```
