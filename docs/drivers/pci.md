---
title: "PCI"
order: 5
---

# Controlador PCI - Configuracion del bus periferico (include/kernel/pci.h / src/kernel/pci.c)

## Que es PCI?

**PCI (Peripheral Component Interconnect)** es un bus de datos que conecta los dispositivos (tarjetas de video, teclados, discos) al CPU/motherboard. En lugar de usar direcciones de memoria arbitrarias, los dispositivos PCI exponen un **espacio de configuracion**: un conjunto de registros (256 bytes) accesibles mediante dos puertos de E/S del x86.

## Por que necesita DemOS un controlador PCI?

DemOS usa PCI para:

1. **Detectar dispositivos** conectados al bus (teclado, raton, VGA, ...).
2. **Leer su direccion de E/S** (I/O ports) a través de los **Base Address Registers (BAR)**.
3. **Seleccionar e instalar drivers** compatibles en el `driver_manager_t`.

El escaneo es iniciado desde `main.c` llamando a `select_drivers(&global_driver_manager)` durante la fase de inicializacion del kernel.

## Espacio de configuracion PCI

| Registro | Offset | Tamaño | Descripcion |
|----------|--------|--------|-------------|
| `0x00` | 0 | 16 | `vendor_id` (0x0000/0xFFFF = slot vacio) |
| `0x02` | 2 | 16 | `device_id` |
| `0x08` | 8 | 8 | `revision_id` |
| `0x09` | 9 | 8 | `interface_id` |
| `0x0A` | 10 | 8 | `sub_class_id` |
| `0x0B` | 11 | 8 | `class_id` |
| `0x0E` | 14 | 8 | `header_type` (bit 7 = multifuncion) |
| `0x3C` | 60 | 8 | `interrupt` (IRQ asignada) |
| `0x10` | 16 | 32 | `BAR0` (... hasta `BAR5` en 0x24) |

### Puertos de E/S del x86

DemOS configura los registros de configuracion escribiendo un **dword de direccion** (32 bits) en el puerto `0xCF8` y leyendo/escribiendo los **datos** en el puerto `0xCFC`.

```
Direccion (0xCF8):  [31]  [30:24]  [23:16]  [15:11]  [10:8]  [7:2]     [1:0]
                    Enable  0       Bus      Device   Funct   Reg offset  0
                                              (0-255)  (0-31)   (0-255)  
```

- **Bus (8 bits)**: numero de bus PCI (0-255).
- **Device (5 bits)**: numero de dispositivo en el slot (0-31).
- **Function (3 bits)**: funcion del dispositivo (0-7); usado por dispositivos multifuncion.
- **Register offset (6 bits, alineado a 4)**: registro dentro del dispositivo (0-255, siempre multiplo de 4).

## include/kernel/pci.h - API

```c
uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t funtion, uint32_t register_offset);
uint32_t pci_write(uint16_t bus, uint16_t device, uint16_t funtion,
                   uint32_t register_offset, uint32_t value);
uint8_t device_has_functions(uint32_t bus, uint16_t device);
void select_drivers(driver_manager_t *manager);
void get_device_descriptor(uint16_t bus, uint16_t device, uint16_t funtion,
                           pci_device_descriptor *out);
base_address_register get_address_register(uint16_t bus, uint16_t device,
                                           uint16_t funtion, uint16_t bar);
driver_t *get_driver(pci_device_descriptor *device);
```

```c
typedef enum { memery_mapping = 0, input_output = 1 } base_address_register_type;

typedef struct {
    uint8_t prefetchable;
    uint8_t *address;
    uint32_t size;
    base_address_register_type type;
} base_address_register;

typedef struct {
    uint32_t port_base;
    uint32_t interrupt;
    uint16_t bus, device, funtion;
    uint16_t vendor_id, device_id;
    uint8_t class_id, sub_class_id, interface_id, revision;
} pci_device_descriptor;
```

| Funcion | Proposito |
|---------|-----------|
| `pci_read()` | Lee un dword del espacio de configuracion a traves de los puertos 0xCF8/0xCFC |
| `pci_write()` | Escribe un dword en el espacio de configuracion |
| `device_has_functions()` | Comprueba el bit 7 de `header_type` (dispositivo multifuncion) |
| `get_device_descriptor()` | Rellena un `pci_device_descriptor` con vendor/device/class/interrupt |
| `get_address_register()` | Lee un BAR (0-5): determina tipo (memoria/IO) y direccion |
| `get_driver()` | Identifica el dispositivo y retorna un `driver_t*` compatible (o NULL) |
| `select_drivers()` | Escanea todo el bus PCI y registra drivers al administrador |

## src/kernel/pci.c - Implementacion

### `pci_read()` - Lectura de configuracion

```c
uint32_t id = (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) |
              ((funtion & 0x07) << 8) | (register_offset & 0xFC);
outl(PCI_COMMAND_PORT, id);       // 0xCF8: direccion
uint32_t result = inl(PCI_DATA_PORT);  // 0xCFC: datos
return result >> (8 * (register_offset % 4));
```

El offset se enmascara con `0xFC` para alinearlo a 4 bytes (los 4 bytes del dword). Como se puede pedir cualquier offset (0-255), el resultado se desplaza para extraer el dword solicitado (byte/half/word dentro del dword).

### `device_has_functions()` - Multifuncion

```c
return pci_read(bus, device, 0, 0x0E) & (1 << 7);
```

Si el bit 7 del `header_type` (registro 0x0E) esta a `1`, el dispositivo soporta hasta 8 funciones (0-7); en caso contrario solo tiene la funcion 0.

### `get_address_register()` - Base Address Register (BAR)

```c
uint32_t header_type = pci_read(bus, device, funtion, 0x0E) & 0x7F;
uint32_t max_bars = 6 - (4 * header_type);
if (bar >= max_bars)
    return result;

uint32_t bar_value = pci_read(bus, device, funtion, 0x10 + (4 * bar));
result.type = (bar_value & 0x1) ? input_output : memery_mapping;
```

- El `header_type` (sin el bit multifuncion) determina cuantos BARs son validos. Para dispositivos normales (`header_type == 0`), hay 6 BARs (0-5); para puentes PCI (`header_type == 1`), solo 2.
- El **bit 0** del valor del BAR indica el tipo: `1` = **I/O** (puerto de E/S), `0` = **memoria mapeada**.
- Para **I/O**, la direccion del puerto es `bar_value & ~0x3` (los 2 bits bajos son flags). El `address` del `base_address_register` se asigna con este valor y se propaga a `pci_device_descriptor.port_base`.
- El `size` y el modo (32/64-bit memoria) quedan como TODO para futura deteccion: el esquema del commit original usa un `switch` de marcador de posicion para los modos `0` (32 bit), `1` (20 bit) y `2` (64 bit).

### `get_driver()` - Identificacion de dispositivos

La identificacion se basa en `vendor_id` y `class_id`:

```c
switch (device->vendor_id) {
case 0x1022: /* AMD */
    switch (device->device_id) {
    case 0x2000: /* am79c973 */
        serial_write_string(..., "[PCI] AMD am79c973 ", 19);
        break;
    default:
        break;
    }
    break;
default: /* Intel (0x8086) and others: no driver yet */
    break;
}
```

| vendor_id | Fabricante | Comentario |
|-----------|------------|------------|
| `0x1022` | AMD | Se reconoce `0x2000` (am79c973) |
| `0x8086` | Intel | Reservado (sin driver aun) |
| `0x10EC` | Realtek | No reconocido todavia |
| `0x100B` | National Semiconductor | No reconocido todavia |

```c
switch (device->class_id) {
case 0x03: /* graphics */
    switch (device->sub_class_id) {
    case 0x00: /* VGA */
        serial_write_string(..., "[PCI] VGA ", 10);
        break;
    default:
        break;
    }
    break;
default:
    break;
}
```

| class_id | Subclase (sub_class_id) | Dispositivo |
|----------|--------------------------|-------------|
| `0x03` | `0x00` | VGA/graphics controller |
| `0x01` | `0x01`/`0x02` | Controlador SATA/IDE |
| `0x0C` | `0x03` | USB controller |

`get_driver()` **retorna siempre NULL** por ahora: es un esqueleto que identifica e imprime el dispositivo, pero aun no construye ningun `driver_t`. Esto permite extenderlo anadiendo casos concretos.

### `select_drivers()` - Enumeracion del bus

El escaneo recorre todos los buses (0-255), dispositivos (0-31) y funciones. La logica de un `(bus, device, function)` se extrajo a `select_drivers_for_function()` para mantener la funcion principal legible:

```
select_drivers(manager)
   │
   ├─ scan bus 0..255
   │   └─ scan device 0..31
   │       ├─ vid = pci_read(bus, device, 0, 0x00)
   │       ├─ si vid == 0x0000 o 0xFFFF: continue (slot vacio)
   │       ├─ num_functions = multifuncion ? 8 : 1
   │       └─ scan funcion 0..num_functions
   │           └─ select_drivers_for_function(bus, device, function, manager)
   │               ├─ get_device_descriptor()
   │               ├─ si vendor_id invalido: return
   │               ├─ por bar_num en 0..5:
   │               │   ├─ get_address_register()
   │               │   ├─ si IO: pci_device.port_base = bar.address
   │               │   └─ driver = get_driver(&pci_device)
   │               │       └─ si driver: driver_manager_add(manager, driver)
   │               └─ serial_write_string("[PCI] B:.. D:.. F:.. V:.. DEV:..\n")
   └─ serial_write_string("[PCI] scan done\n")
```

## Flujo completo: deteccion de un dispositivo PCI

```
1. main.c llama select_drivers(&global_driver_manager)
2. select_drivers escanea bus/device/funcion
3. get_device_descriptor lee vendor_id/device_id/class_id/...
4. para cada BAR (0-5):
      get_address_register determina tipo y direccion
      si es I/O: se asigna port_base al descriptor
      get_driver identifica el dispositivo (p.ej. VGA) → NULL
5. (futuro) get_driver construye un driver_t y lo registra
6. driver_manager_activate_all() activa los drivers en main.c
```

## Dependencias

| Modulo | Funcion usada |
|--------|---------------|
| `drivers/driver.h` | `driver_t`, `driver_manager_t`, `driver_manager_add()` |
| `drivers/serial.h` | `serial_write_string()`, `serial_write_hex16()`, `serial_write_u8()` |
| `asm.h` | `outl()`, `inl()` |
