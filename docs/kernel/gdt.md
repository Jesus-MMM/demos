---
title: "GDT"
order: 4
---

# Tabla de Descriptores Globales - GDT (include/kernel/gdt.h / src/kernel/gdt.c)

## Que es la GDT?

La **GDT (Global Descriptor Table)** es una tabla de 8 entradas que define los **segmentos de memoria** en modo protegido x86. Cada entrada (descriptor) describe un segmento: su direccion base, limite (tamaño maximo), y atributos de acceso (lectura, escritura, ejecucion, nivel de privilegio).

Sin una GDT, el CPU no puede acceder a la memoria en modo protegido porque no tiene forma de validar las direcciones.

## Por que necesita DemOS una GDT?

Aunque GRUB ya configura un modo protegido basico, DemOS crea su propia GDT para:

1. **Control total** sobre los segmentos de memoria
2. **Soporte de interrupciones** (el IDT requiere un selector de segmento de codigo valido en la GDT)
3. **Buenas practicas** de desarrollo de kernels

## Estructura de un descriptor de segmento

Cada descriptor de la GDT es una estructura de **8 bytes** (64 bits):

```
 63                    48 47           40 39    32
+------------------------+--------------+--------+
|  Base (high 8 bits)   | Access Byte  | Flags+ |  Byte 7-4
+------------------------+--------------+--------+
 31                    16 15           0
+------------------------+-----------------------+
|  Base (mid 8 bits)    |   Base (low 16 bits)   |  Byte 3-0
+------------------------+-----------------------+

 31                    16 15           0
+------------------------+-----------------------+
|  Flags+Limit(high 4)  |   Limit (low 16 bits)  |  (otros bytes)
+------------------------+-----------------------+
```

### Campo `base` (32 bits)

Direccion de inicio del segmento en memoria fisica. En DemOS, los segmentos de codigo y datos usan base `0x00000000` y limite `0xFFFFFFFF` (todo el espacio de 4 GB).

### Campo `limit` (20 bits)

Tamaño maximo del segmento. Se codifica en 20 bits con un factor de granularidad:

| Granularidad | Calculo del limite |
|-------------|---------------------|
| Byte (G=0) | Limite = valor directo (hasta 1 MB) |
| Pagina (G=1) | Limite = valor × 4096 (hasta 4 GB) |

### Campo `access` (8 bits)

Define las permisos y tipo del segmento:

| Bit | Nombre | Significado |
|-----|--------|-------------|
| 7 | P (Present) | 1 = segmento presente en memoria |
| 6-5 | DPL | Nivel de privilegio (0 = kernel, 3 = usuario) |
| 4 | S | 0 = segmento de sistema, 1 = codigo/datos |
| 3 | E | 1 = segmento de codigo, 0 = segmento de datos |
| 2 | DC | Codigo: conformante. Datos: direction |
| 1 | RW | Codigo: legible. Datos: escribible |
| 0 | Accessed | CPU lo pone a 1 cuando accede al segmento |

### Valores de `access` usados en DemOS

| Valor | Binario | Tipo |
|-------|---------|------|
| `0x9A` | `10011010` | Codigo: presente, DPL=0, ejecutable, legible |
| `0x92` | `10010010` | Datos: presente, DPL=0, escribible |

### Campo `flags` (4 bits)

| Bit | Nombre | Significado |
|-----|--------|-------------|
| 3 | G | Granularidad: 0=bytes, 1=paginas de 4KB |
| 2 | D/B | 0=16 bits, 1=32 bits |
| 1 | L | 0=modo legado, 1=modo long (64 bits, no usado) |
| 0 | AVL | Reservado para uso del SO |

En DemOS: `G=1, D=1` → granularidad de 4KB, modo 32 bits.

## Estructura de la GDT de DemOS

```c
typedef struct __attribute__((packed)) {
    SegmentDescriptor null_segment_selector;      // Entrada 0: Nula (obligatoria)
    SegmentDescriptor unused_segment_selector;    // Entrada 1: No usada
    SegmentDescriptor code_segment_selector;      // Entrada 2: Codigo (0x10)
    SegmentDescriptor data_segment_selector;      // Entrada 3: Datos (0x18)
} global_descriptor_table;
```

| Entrada | Selector | Tipo | Base | Limite | Access |
|---------|----------|------|------|--------|--------|
| 0 | `0x00` | Nula | 0 | 0 | 0x00 |
| 1 | `0x08` | Sin usar | 0 | 0 | 0x00 |
| 2 | `0x10` | Codigo | `0x00000000` | `0xFFFFFFFF` | `0x9A` |
| 3 | `0x18` | Datos | `0x00000000` | `0xFFFFFFFF` | `0x92` |

### Selectores de segmento

Los selectores son los offsets de cada descriptor dentro de la GDT:

```c
#define GDT_CODE_SELECTOR 0x10   // Entrada 2 (offset 16 bytes)
#define GDT_DATA_SELECTOR 0x18   // Entrada 3 (offset 24 bytes)
```

Estos valores se cargan en los registros de segmento del CPU (`CS`, `DS`, `ES`, `FS`, `GS`, `SS`).

## src/kernel/gdt.c - Implementacion

### `init_gdt()` - Inicializacion completa

```c
void init_gdt(global_descriptor_table *gdt)
{
    // 1. Llenar los 4 descriptores
    init_segment_descriptor(&gdt->null_segment_selector, 0, 0, 0);
    init_segment_descriptor(&gdt->unused_segment_selector, 0, 0, 0);
    init_segment_descriptor(&gdt->code_segment_selector, 0, 0xFFFFFFFF, 0x9A);
    init_segment_descriptor(&gdt->data_segment_selector, 0, 0xFFFFFFFF, 0x92);

    // 2. Cargar GDTR con LGDT
    gdtr_t gdtr;
    gdtr.base = (uint32_t)gdt;
    gdtr.limit = sizeof(global_descriptor_table) - 1;
    __asm__ volatile("lgdt %0" : : "m"(gdtr));

    // 3. Recargar CS con far jump
    __asm__ volatile("ljmp %0, $1f\n1:\n" : : "i"(GDT_CODE_SELECTOR));

    // 4. Recargar registros de segmento de datos
    __asm__ volatile(
        "mov %0, %%ds\nmov %0, %%es\n"
        "mov %0, %%fs\nmov %0, %%gs\nmov %0, %%ss\n"
        : : "r"((uint16_t)GDT_DATA_SELECTOR)
    );
}
```

**Pasos criticos:**

| Paso | Que hace | Por que es necesario |
|------|----------|----------------------|
| 1. Llenar descriptores | Define 4 segmentos | La GDT debe estar poblada antes de usarla |
| 2. LGDT | Carga la GDTR (Direccion + Tamaño) | El CPU necesita saber donde esta la GDT |
| 3. Far jump (`ljmp`) | Recarga el registro CS | `CS` conserva su valor anterior; solo un `ljmp` lo actualiza |
| 4. Recargar DS/ES/etc | Actualiza registros de datos | Deben apuntar al nuevo selector de datos |

### `init_segment_descriptor()` - Configuracion individual

```c
void init_segment_descriptor(SegmentDescriptor *sd, uint32_t base,
                             uint32_t limit, uint8_t type)
{
    if (limit <= 65536) {
        sd->flags_and_limit_high = 0x40;        // G=0 (bytes), D=1 (32 bits)
    } else {
        if ((limit & 0xFFF) != 0xFFF)
            limit = (limit >> 12) - 1;
        else
            limit = limit >> 12;
        sd->flags_and_limit_high = 0xC0;        // G=1 (paginas), D=1 (32 bits)
    }
    sd->limit_low = limit & 0xFFFF;
    sd->flags_and_limit_high |= (limit >> 16) & 0xF;
    sd->base_low = base & 0xFFFF;
    sd->base_mid = (base >> 16) & 0xFF;
    sd->base_high = (base >> 24) & 0xFF;
    sd->access_byte = type;
}
```

**Logica de granularidad:**

```
Si limite <= 65536:
    Usar granularidad de BYTE (G=0)
    Limite directo en los 20 bits

Si limite > 65536:
    Convertir a paginas de 4KB (dividir por 4096)
    Si (limite & 0xFFF) != 0xFFF:
        Restar 1 para cubrir los bytes parciales
    Activar G=1
```

### Funciones auxiliares

```c
uint32_t segment_descriptor_get_base(SegmentDescriptor *sd);
uint32_t segment_descriptor_get_limit(SegmentDescriptor *sd);
uint16_t gdt_get_code_selector(global_descriptor_table *gdt);
uint16_t gdt_get_data_selector(global_descriptor_table *gdt);
```

| Funcion | Proposito |
|---------|-----------|
| `get_base` | Extrae la base de 32 bits desde un descriptor |
| `get_limit` | Extrae el limite de 20 bits, ajustando por granularidad |
| `get_code_selector` | Calcula el offset del descriptor de codigo en la GDT |
| `get_data_selector` | Calcula el offset del descriptor de datos en la GDT |

## GDTR - Registro de Descriptor Global

```c
typedef struct __attribute__((packed)) {
    uint16_t size;     // Tamaño de la GDT - 1
    uint32_t base;     // Direccion base de la GDT
} gdtr_t;
```

El registro GDTR tiene 48 bits: 16 bits de tamaño + 32 bits de direccion. La instruccion `LGDT` carga este registro.

## Flujo de inicializacion

```
init_gdt()
    │
    ├─ init_segment_descriptor(null)      → Entrada 0 (nula)
    ├─ init_segment_descriptor(unused)    → Entrada 1 (sin usar)
    ├─ init_segment_descriptor(code)      → Entrada 2 (base=0, limite=4GB)
    └─ init_segment_descriptor(data)      → Entrada 3 (base=0, limite=4GB)
           │
           ├─ Si limite <= 65536: G=0 (bytes)
           └─ Si limite > 65536:  G=1 (paginas de 4KB)
                  │
                  ├─ limit_low = limite & 0xFFFF
                  └─ flags |= (limite >> 16) & 0xF
    │
    ├─ LGDT gdtr (cargar la tabla al CPU)
    ├─ ljmp GDT_CODE_SELECTOR (recargar CS)
    └─ mov GDT_DATA_SELECTOR → DS/ES/FS/GS/SS
```
