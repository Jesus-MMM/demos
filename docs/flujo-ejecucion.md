# Flujo de ejecucion completo

Este documento traza el recorrido del codigo desde que el CPU se enciende hasta que el mensaje aparece en pantalla.

## Diagrama de secuencia

```
ENCENDIDO
    |
    v
BIOS (firmware)
    |  Busca dispositivo booteable
    v
GRUB (bootloader)
    |  Lee grub.cfg, busca cabecera Multiboot en kernel.elf
    |  Cambia a modo protegido (32 bits)
    v
loader.s (ensamblador)
    |  Configura la pila (ESP)
    |  Llama a kernel_main()
    v
kernelmain.c (kernel)
    |  Llama a write_to_screen("Welcome to DemOS", 14)
    v
io.c / io.h
    |  write_letter_to_buffer() escribe en 0xB8000
    |  move_cursor() actualiza el cursor via puertos CRTC
    v
PANTALLA VGA
    |  "Welcome to DemOS" en blanco sobre negro
    v
BUCLE INFINITO (loader.s .hang)
```

## Paso a paso detallado

### 1. Encendido / Reset

- El CPU comienza ejecutando codigo en la direccion `0xFFFFFFF0` (BIOS).
- La BIOS realiza el POST (Power-On Self Test) y busca un dispositivo booteable.

### 2. GRUB (bootloader)

- La BIOS encuentra la ISO de DemOS y carga GRUB.
- GRUB lee `grub.cfg`:
  ```cfg
  menuentry "DemOS" {
      multiboot /boot/kernel.elf
      boot
  }
  ```
- GRUB localiza la **cabecera Multiboot** en el kernel (en la direccion `0x00100000`):
  - Verifica que `MAGIC_NUMBER = 0x1BADB002`.
  - Verifica que `MAGIC_NUMBER + FLAGS + CHECKSUM = 0`.
- GRUB cambia el CPU a **modo protegido (32 bits)** y salta a la direccion de `loader`.

### 3. loader.s (Assembly)

```nasm
loader:
    mov esp, kernel_stack + 4096    ; Configurar pila
    call kernel_main                 ; Llamar al kernel C
.hang:
    jmp .hang                        ; Bucle infinito
```

- **ESP** apunta al final de un buffer de 4 KB.
- La instruccion `call` empuja la direccion de retorno en la pila y salta a `kernel_main`.
- Si `kernel_main` retorna, el CPU queda atrapado en `.hang`.

### 4. kernel_main.c (C)

```c
int kernel_main()
{
    const char *text = "Welcome to DemOS";
    write_to_screen(text, strlen(text));
    return 0;
}
```

- La cadena `"Welcome to DemOS"` se almacena en la seccion `.rodata`.
- `strlen()` cuenta 14 caracteres.
- `write_to_screen()` inicia la escritura en pantalla.

### 5. io.c (Framebuffer VGA)

```c
void write_letter_to_buffer('W', 0, 0, WHITE, BLACK)
{
    // attribute = (0x0 << 4) | (0xF & 0x0F) = 0x0F
    // char_with_attr = (0x0F << 8) | 0x57 = 0x0F57
    // framebuffer[0] = 0x0F57
}
```

Para cada letra:
1. Calcula el atributo (fondo negro + texto blanco = `0x0F`).
2. Combina con el caracter ASCII.
3. Escribe en `framebuffer[posicion]`.
4. La tarjeta VGA lee esta memoria 60 veces por segundo y la muestra en pantalla.

### 6. Actualizacion del cursor

```c
move_cursor(14)
{
    // pos = 14, low = 0x0E, high = 0x00
    outb(0x3D4, 0x0E)    // Seleccionar registro: byte alto del cursor
    outb(0x3D5, 0x00)    // Escribir byte alto
    outb(0x3D4, 0x0F)    // Seleccionar registro: byte bajo del cursor
    outb(0x3D5, 0x0E)    // Escribir byte bajo
}
```

### 7. Estado final

```
Pantalla:
┌──────────────────────────────────────────────┐
│ Welcome to DemOS                             │ <- Fila 0
│                                              │
│ (25 filas de espacio vacio)                  │
│                                              │
└──────────────────────────────────────────────┘
Cursor parpadeando despues de la 'S' de "DemOS"
CPU en bucle infinito en loader.s:.hang
```

## Resumen de tecnologias involucradas

| Tecnologia | Uso en DemOS |
|------------|--------------|
| **x86 Assembly (NASM)** | Cabecera Multiboot, configuracion de pila |
| **C (GCC)** | Logica del kernel y librerias |
| **GRUB** | Gestor de arranque |
| **VGA Text Mode** | Visualizacion en pantalla |
| **Puertos E/S (x86)** | Control de cursor y scroll via CRTC |
| **Framebuffer** | Escritura directa en memoria de video |
| **Linker Script** | Organizacion de secciones en memoria |
| **Makefile** | Automatizacion de compilacion |
| **QEMU** | Emulacion para pruebas |
| **Multiboot spec** | Estandar de interfaz kernel-bootloader |

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [VGA y Framebuffer](vga-framebuffer.md) | [Volver al inicio](introduccion.md) |

---

**Fin de la documentacion de DemOS**
