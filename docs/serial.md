# Puerto serie - UART 16550 (include/drivers/serial.h / src/drivers/serial.c)

## Que es el puerto serie?

El **puerto serie** (UART - Universal Asynchronous Receiver/Transmitter) es un canal de comunicacion bidireccional que transmite datos un bit a la vez. En desarrollo de kernels, el puerto serie es la herramienta de depuracion mas fiable porque funciona sin drivers de video ni sistemas de archivos.

DemOS usa el chip **16550** (o compatible) en el puerto **COM1** (`0x3F8`) para enviar mensajes de depuracion a traves de QEMU (`-serial stdio`).

## Por que usar el puerto serie?

| Ventaja | Explicacion |
|---------|-------------|
| Sin dependencias | No necesita framebuffer, drivers de video ni sistema de archivos |
| Siempre disponible | El hardware UART existe en todas las placas x86 |
| Output visible | QEMU redirige el serie a la terminal con `-serial stdio` |
| Simple | Solo escribe bytes a un puerto de E/S |

## Mapa de memoria del UART

DemOS usa **COM1** (base `0x3F8`) y **COM2** (base `0x2F8`):

| Puerto COM1 | Puerto COM2 | Registro |
|-------------|-------------|----------|
| `0x3F8` | `0x2F8` | Data (TX/RX) |
| `0x3F9` | `0x2F9` | Interrupt Enable |
| `0x3FA` | `0x2FA` | FIFO Control |
| `0x3FB` | `0x2FB` | Line Control |
| `0x3FD` | `0x2FD` | Line Status |

### Registros

| Registro | Proposito |
|----------|-----------|
| **Data** | Envia/recibe un byte |
| **Interrupt Enable** | Habilita/deshabilita interrupciones del UART |
| **FIFO Control** | Activa/deshabilita la cola FIFO de 16 bytes |
| **Line Control** | Configura formato: bits de datos, paridad, stop bits |
| **Line Status** | Estado de transmision y recepcion |

## serial.h - Definiciones

```c
#define COM1_BASE_ADDRESS 0x3F8
#define COM2_BASE_ADDRESS 0x2F8

#define SERIAL_DATA_PORT(base_address)        ((base_address))
#define SERIAL_INTERRUPT_PORT(base_address)   ((base_address) + 1)
#define SERIAL_FIFO_CMD_PORT(base_address)    ((base_address) + 2)
#define SERIAL_LINE_CMD_PORT(base_address)    ((base_address) + 3)
#define SERIAL_LINE_STATUS_PORT(base_address) ((base_address) + 5)
```

Las macros calculan la direccion de cada registro a partir de la base del puerto.

## serial.c - Implementacion

### `serial_init()` - Inicializacion del UART

```c
void serial_init(uint16_t com)
{
    outb(SERIAL_INTERRUPT_PORT(com), 0x00);   // Desactivar interrupciones
    outb(SERIAL_LINE_CMD_PORT(com), 0x80);    // DLAB = 1 (acceder a divisor)
    outb(SERIAL_DATA_PORT(com), 1 & 0xFF);    // Divisor bajo (baud rate)
    outb(SERIAL_DATA_PORT(com) + 1, 0);       // Divisor alto
    outb(SERIAL_LINE_CMD_PORT(com), 0x03);    // 8N1 (8 datos, sin paridad, 1 stop)
    outb(SERIAL_FIFO_CMD_PORT(com), 0xC7);    // Habilitar FIFO, 14 bytes
}
```

**Pasos de inicializacion:**

| Paso | Que hace |
|------|----------|
| 1. Desactivar interrupciones | Escribe `0x00` al registro de interrupciones |
| 2. Activar DLAB | Bit 7 del registro Line Control = 1 para acceder al divisor de baudios |
| 3. Establecer baud rate | Divisor = 1 → 115200 baudios (velocidad maxima) |
| 4. Configurar Line | `0x03` = 8 bits de datos, sin paridad, 1 bit de stop (8N1) |
| 5. Habilitar FIFO | `0xC7` = activar FIFO, umbral de 14 bytes |

### Formato 8N1

```
[Start][D0][D1][D2][D3][D4][D5][D6][D7][Stop]
  1b    ---- 8 bits de datos ----        1b
```

| Campo | Valor |
|-------|-------|
| Data bits | 8 |
| Parity | Ninguna |
| Stop bits | 1 |

### `serial_write()` - Envio de un byte

```c
void serial_write(uint16_t com, uint8_t data)
{
    while (!is_transmition_buffer_empty(com)) {}
    outb(SERIAL_DATA_PORT(com), data);
}
```

**Espera activa** hasta que el buffer de transmision este vacio, luego escribe el byte en el registro Data.

### `serial_write_string()` - Envio de cadena

```c
void serial_write_string(uint16_t com, const char *buffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        serial_write(com, buffer[i]);
}
```

Envia `len` bytes del buffer, uno por uno usando `serial_write()`.

### `serial_read()` - Recepcion de un byte

```c
char serial_read(uint16_t com)
{
    while (!is_data_received(com)) {}
    return (char)inb(SERIAL_DATA_PORT(com));
}
```

**Bloqueante**: espera hasta que haya datos disponibles en el buffer de recepcion.

### `is_transmition_buffer_empty()` - Verificacion de estado

```c
int8_t is_transmition_buffer_empty(uint16_t com)
{
    return (inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20) == 0x20;
}
```

Lee el registro **Line Status** y verifica el bit 5 (THRE - Transmitter Holding Register Empty). Retorna `1` si el buffer de transmision esta listo para recibir datos.

### `is_data_received()` - Verificacion de recepcion

```c
int8_t is_data_received(uint16_t com)
{
    return (inb(SERIAL_LINE_STATUS_PORT(com)) & 0x01) == 0x01;
}
```

Verifica el bit 0 (DR - Data Ready). Retorna `1` si hay datos esperando ser leidos.

## Registro Line Status (LSR)

| Bit | Nombre | Significado |
|-----|--------|-------------|
| 0 | DR | Data Ready - hay datos para leer |
| 1 | OE | Overrun Error - dato perdido |
| 2 | PE | Parity Error |
| 3 | FE | Framing Error |
| 4 | BI | Break Interrupt |
| 5 | THRE | Transmitter Holding Register Empty |
| 6 | TEMT | Transmitter Empty |

## Uso desde QEMU

Para ver la salida del serie en la terminal:

```bash
qemu-system-i386 -cdrom DemOS.iso -serial stdio
```

Todos los `serial_write_string()` se imprimen en la terminal donde se ejecuto QEMU.

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [Librerias del sistema](librerias.md) | [GDT](gdt.md) |

| Relacionados |
|--------------|
| [Sistema de interrupciones](interrupts.md) |
| [Driver de teclado](keyboard.md) |
