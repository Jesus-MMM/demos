# Driver de mouse PS/2 (include/drivers/mouse.h / src/drivers/mouse.c)

## Que es el driver de mouse?

El **driver de mouse** maneja las interrupciones generadas por el mouse PS/2 (IRQ 12, interrupcion `0x2C`), lee los **paquetes de 3 bytes** del puerto de datos `0x60` y los traduce a movimiento y botones. Actualmente implementa un cursor visual que invierte los colores de la pantalla en la posicion del mouse.

## El mouse PS/2

El mouse PS/2 envia paquetes de **3 bytes** cada vez que se mueve o se presiona/suelta un boton:

```
Byte 0: Botones + flags
  Bit 0: Boton izquierdo
  Bit 1: Boton derecho
  Bit 2: Boton central
  Bit 3: Siempre 1
  Bit 4: Movimiento X negativo (signo)
  Bit 5: Movimiento Y negativo (signo)
  Bit 6: Overflow X
  Bit 7: Overflow Y

Byte 1: Movimiento X (offset desde ultimo paquete)
Byte 2: Movimiento Y (offset desde ultimo paquete)
```

## mouse.h - API

```c
void mouse_init(void);
uint32_t mouse_handler(uint32_t esp);
```

| Funcion | Proposito |
|---------|-----------|
| `mouse_init()` | Habilita el mouse PS/2 y configura el controlador |
| `mouse_handler()` | Manejador de IRQ 12, llamado desde `handle_interrupt()` |

## mouse.c - Implementacion

### Variables globales

```c
static uint8_t buffer[3];    // Buffer de 3 bytes para el paquete
static uint8_t offset;       // Byte actual del paquete (0-2)
static uint8_t buttons;      // Estado previo de los botones
```

### `mouse_init()` - Activacion del mouse

```c
void mouse_init(void)
{
    offset = 0;
    buttons = 0;

    // 1. Drenar buffer de datos
    while (inb(MS_COMMAND_PORT) & 0x1)
        inb(MS_DATA_PORT);

    // 2. Habilitar mouse PS/2
    outb(MS_COMMAND_PORT, 0xA8);

    // 3. Leer byte de configuracion del controlador
    outb(MS_COMMAND_PORT, 0x20);
    while (!(inb(MS_COMMAND_PORT) & 0x1)) {}
    uint8_t status = (inb(MS_DATA_PORT) | 2);

    // 4. Escribir nueva configuracion
    outb(MS_COMMAND_PORT, 0x60);
    while (inb(MS_COMMAND_PORT) & 0x2) {}
    outb(MS_DATA_PORT, status);

    // 5. Habilitar escaneo del mouse (0xF4 via puerto 0xD4)
    outb(MS_COMMAND_PORT, 0xD4);
    while (inb(MS_COMMAND_PORT) & 0x2) {}
    outb(MS_DATA_PORT, 0xF4);
}
```

**Paso a paso:**

| Paso | Que hace | Por que |
|------|----------|---------|
| 1. Drenar buffer | Leer y descartar datos pendientes | Evitar paquetes residuales |
| 2. `0xA8` al `0x64` | Habilitar el segundo puerto PS/2 (mouse) | Activar hardware del mouse |
| 3. `0x20` al `0x64` | Leer byte de configuracion del controlador | Obtener estado actual |
| 4. `\| 2` | Bit 1 = 1 (habilitar IRQ12) | Permitir que el mouse genere interrupciones |
| 5. `0x60` al `0x64` | Seleccionar registro de configuracion | Indicar que el siguiente dato es configuracion |
| 6. `status` al `0x60` | Escribir nueva configuracion | Aplicar los cambios |
| 7. `0xD4` al `0x64` | Indicar que el siguiente dato es para el mouse | Los comandos del mouse van via 0xD4 |
| 8. `0xF4` al `0x60` | Habilitar escaneo del mouse | El mouse empezara a enviar paquetes |

> **Nota**: El comando `0xD4` es necesario porque el puerto de datos `0x60` esta compartido entre teclado y mouse. Sin `0xD4`, el `0xF4` se enviaria al teclado en lugar del mouse.

### `mouse_handler()` - Manejo de interrupciones

```c
uint32_t mouse_handler(uint32_t esp)
{
    uint8_t status = inb(MS_COMMAND_PORT);
    if (!(status & 0x20))        // Bit 5: datos disponibles para mouse
        return esp;

    static int8_t x = 0;
    static int8_t y = 0;

    buffer[offset] = inb(MS_DATA_PORT);
    offset = (offset + 1) % 3;

    if (offset == 0) {           // Paquete completo (3 bytes)
        // Invertir colores en posicion anterior (cursor visual)
        video_memory[(80 * y) + x] = /* swap fg/bg */;

        // Actualizar posicion
        x = (int8_t)(x + buffer[1]);    // Byte 1: movimiento X
        y = (int8_t)(y - buffer[2]);    // Byte 2: movimiento Y

        // Clamping a limites de pantalla
        if (x < 0) x = 0;
        if (x >= 80) x = 79;
        if (y < 0) y = 0;
        if (y >= 25) y = 24;

        // Invertir colores en nueva posicion (cursor visual)
        video_memory[(80 * y) + x] = /* swap fg/bg */;
    }

    // Detectar cambios en botones (para futuro uso)
    for (uint8_t i = 0; i < 3; i++) {
        if ((buffer[0] & (0x01 << i)) != (buttons & (0x01 << i))) {
            /* TODO: CREATE ALL THE MOUSE CLICK LOGICS */
        }
    }
    buttons = buffer[0];

    return esp;
}
```

**Flujo:**

```
mouse_handler(esp)
    │
    ├─ Leer status del controlador (0x64)
    ├─ Si bit 5 no esta set: no hay datos → retornar esp
    ├─ Leer byte del paquete de 0x60
    ├─ offset = (offset + 1) % 3
    ├─ Si offset == 0 (paquete completo):
    │   ├─ Invertir colores en posicion anterior
    │   ├─ Actualizar x/y con buffer[1] y buffer[2]
    │   ├─ Clampear a limites de pantalla (80x25)
    │   └─ Invertir colores en nueva posicion
    ├─ Detectar cambios de botones (futuro)
    └─ Retornar esp
```

### Cursor visual

El driver implementa un cursor visual simple que **invierte los colores** (foreground ↔ background) de la celda VGA en la posicion del mouse:

```
Posicion anterior: swap fg/bg → restaurar celda original
Nueva posicion:    swap fg/bg → resaltar celda actual
```

Esto crea un efecto de "cursor" sin usar un caracter especial.

### Clamping de posicion

Las coordenadas `x` e `y` son `int8_t` (rango -128 a 127) para manejar movimientos negativos. Se clampean a los limites de la pantalla:

| Coordenada | Minimo | Maximo |
|------------|--------|--------|
| `x` (columna) | 0 | 79 |
| `y` (fila) | 0 | 24 |

### Deteccion de botones (futuro)

El driver detecta cambios en los botones comparando el estado actual con el anterior:

```c
for (uint8_t i = 0; i < 3; i++) {
    if ((buffer[0] & (0x01 << i)) != (buttons & (0x01 << i))) {
        /* TODO: CREATE ALL THE MOUSE CLICK LOGICS */
    }
}
buttons = buffer[0];
```

Bit 0 = izquierdo, Bit 1 = derecho, Bit 2 = central. Actualmente no realiza ninguna accion (TODO).

## Flujo completo: mouse movido

```
1. Usuario mueve el mouse
2. Mouse PS/2 envia paquete de 3 bytes
3. PIC esclavo genera IRQ 12
4. CPU ejecuta handle_interrupt_request0x0C
5. handle_interrupt(0x2C, esp) llama a mouse_handler(esp)
6. mouse_handler lee bytes del puerto 0x60
7. Con 3 bytes completos: actualizar posicion, invertir colores
8. handle_interrupt envia EOI al PIC maestro y esclavo
9. CPU reanuda codigo interrumpido
10. Cursor visual se mueve en la pantalla
```

## Dependencias

| Modulo | Funcion usada |
|--------|---------------|
| `asm.h` | `inb()`, `outb()` |
| `drivers/vga.h` | Acceso directo a framebuffer VGA (`0xB8000`) |
| `drivers/serial.h` | `serial_write_string()` (mensaje de activacion) |

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Driver de teclado](keyboard.md) | [Puerto serie](serial.md) |

| Relacionados |
|--------------|
| [Sistema de interrupciones](interrupts.md) |
| [VGA y Framebuffer](vga-framebuffer.md) |
