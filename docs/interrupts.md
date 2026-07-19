# Sistema de interrupciones - IDT / PIC 8259A (interrupts.h / interrupts.c / interruptstubs.s)

## Que es el sistema de interrupciones?

Las **interrupciones** son mecanismos que permiten al CPU responder a eventos (teclado, temporizador, errores) deteniendo el codigo actual, ejecutando un **manejador** (handler) y resumiendo donde quedo. En modo protegido x86, el CPU usa la **IDT (Interrupt Descriptor Table)** para localizar los handlers.

## Componentes del sistema

```
        Hardware                CPU                    Kernel
     +-----------+         +----------+          +-----------+
     | Teclado   |-------->|  IRQ 1   |--------->| keyboard  |
     | PIT Timer |-------->|  IRQ 0   |--------->| (futuro)  |
     | Errores   |-------->| Excep.   |--------->| handler   |
     +-----------+         +----------+          +-----------+
                               |                      |
                           IDT (256 entradas)    Handler en C
                               |                      |
                           PIC 8259A             EOI al PIC
```

## Dos tipos de interrupcion

| Tipo | Vector | Origen | Ejemplo |
|------|--------|--------|---------|
| **Excepcion** | `0x00 - 0x1F` | CPU (errores internos) | Division por cero (0x00), Page Fault (0x0E) |
| **IRQ** (Hardware) | `0x20 - 0x2F` | Dispositivos externos | Teclado (IRQ 1), Timer (IRQ 0) |

## PIC 8259A - Controlador de interrupciones programable

El **PIC 8259A** es un chip (o emulacion en hardware virtual) que gestiona las interrupciones de hardware. DemOS usa **dos PICs** conectados en cascada:

```
IRQ 0-7   → PIC Maestro (0x20-0x27)
IRQ 8-15  → PIC Esclavo  (0x28-0x2F)
              │
              └─ Conectado al IRQ 2 del PIC Maestro
```

### Puertos del PIC

| Puerto | Nombre | Proposito |
|--------|--------|-----------|
| `0x20` | PIC Maestro CMD | Enviar comandos de inicializacion y EOI |
| `0x21` | PIC Maestro Data | Enmascarar IRQs y enviar ICW |
| `0xA0` | PIC Esclavo CMD | Enviar comandos al esclavo |
| `0xA1` | PIC Esclavo Data | Enmascarar IRQs del esclavo |

### Remapeo de IRQs

El PIC por defecto envia IRQs 0-7 como interrupciones 0x08-0x0F, que **colisionan con las excepciones del CPU**. Por eso DemOS las remapea a `0x20-0x27`:

```
Antes:  IRQ 0 → INT 0x08 (colisiona con Doble Fallo)
        IRQ 1 → INT 0x09 (colisiona con GDT not available)

Despues: IRQ 0 → INT 0x20 (timer, sin colision)
         IRQ 1 → INT 0x21 (teclado, sin colision)
```

### Secuencia de inicializacion ICW

DemOS envia las 4 palabras de inicializacion (ICW1-ICW4) a ambos PICs:

```c
// ICW1: Inicializacion, esperar ICW4
outb(0x20, 0x11);   outb(0xA0, 0x11);

// ICW2: Vector base (IRQ 0 → INT 0x20, IRQ 8 → INT 0x28)
outb(0x21, 0x20);   outb(0xA1, 0x28);

// ICW3: Conexiones (Maestro: IRQ 2 → esclavo, Esclavo: IRQ 2)
outb(0x21, 0x04);   outb(0xA1, 0x02);

// ICW4: Modo 8086
outb(0x21, 0x01);   outb(0xA1, 0x01);
```

### Enmascaramiento de IRQs

Despues de ICW, se envia una mascara que decide que IRQs estan habilitadas:

```c
outb(0x21, 0xFC);   // Maestro: solo IRQ 0 y 1 habilitadas
outb(0xA1, 0xFF);   // Esclavo: todas deshabilitadas
```

```
Mascara maestro: 0xFC = 11111100
                        │││││││└── IRQ 0 habilitada (Timer)
                        ││││││└─── IRQ 1 habilitada (Teclado)
                        │││││└──── IRQ 2 deshabilitada
                        ││││└───── IRQ 3 deshabilitada
                  ...       (el resto deshabilitado)
```

## IDT - Tabla de Descriptores de Interrupcion

La **IDT** es una tabla de **256 entradas**, una por cada posible interrupcion. Cada entrada es un descriptor de 8 bytes que apunta al handler en ensamblador.

### Estructura de cada entrada

```c
typedef struct __attribute__((packed)) {
    uint16_t handler_address_low_bits;   // Bits 0-15 de la direccion del handler
    uint16_t gdt_code_segment_selector;  // Selector de segmento de codigo (0x10)
    uint8_t reserved;                    // Siempre 0
    uint8_t access;                      // Tipo de compuerta + privilegios
    uint16_t handler_address_high_bits;  // Bits 16-31 de la direccion del handler
} gate_descriptor;
```

### Campo `access`

| Bit | Nombre | Valor en DemOS | Significado |
|-----|--------|----------------|-------------|
| 7 | P (Present) | 1 | Descriptor valido |
| 6-5 | DPL | 00 | Nivel 0 (solo kernel) |
| 4-3 | Type | 1110 | Compuerta de interrupcion (32 bits) |

Valor completo: `0x8E` = `10001110` (Present=1, DPL=0, Type=0xE)

### Configuracion de entradas

DemOS configura 3 tipos de entradas en la IDT:

| Rango | Handler | Uso |
|-------|---------|-----|
| `0x00 - 0x1F` | `exception_handler_table` | Excepciones del CPU |
| `0x20` | `handle_interrupt_request0x00` | IRQ 0 (Timer) |
| `0x21` | `handle_interrupt_request0x01` | IRQ 1 (Teclado) |
| `0x22 - 0xFF` | `ignore_interrupt_request` | Sin handler (solo EOI) |

## interruptstubs.s - Stubs en ensamblador

Los **interrupt stubs** son funciones en ensamblador que sirven de puente entre el CPU y el manejador en C. Son necesarias porque el CPU:

1. **No puede llamar a C directamente** desde una interrupcion
2. **Necesita guardar el estado** del CPU antes de ejecutar C
3. **Necesita restaurar el estado** y ejecutar `IRET` al retornar

### Macro `handle_interrupt_exception_no_err`

```asm
.macro handle_interrupt_exception_no_err num
.global handle_interrupt_exception\num
handle_interrupt_exception\num:
    pushl $0              ; Push error code dummy (0)
    movl $\num, (interruptnumber)  ; Guardar numero de interrupcion
    jmp int_bottom        ; Saltar al handler comun
.endm
```

Usada para excepciones que **no pushean un error code** (0x00-0x07, 0x09, 0x0F-0x13, etc.).

### Macro `handle_interrupt_exception_err`

```asm
.macro handle_interrupt_exception_err num
handle_interrupt_exception\num:
    movl $\num, (interruptnumber)  ; Guardar numero
    jmp int_bottom                 ; Error code ya esta en la pila
.endm
```

Usada para excepciones que **si pushean un error code** (0x08, 0x0A-0x0E, 0x11).

### Macro `handle_interrupt_request`

```asm
.macro handle_interrupt_request num
handle_interrupt_request\num:
    pushl $0              ; Dummy error code
    movl $\num + IRO_BASE, (interruptnumber)  ; IRQ + 0x20 = numero de interrupcion
    jmp int_bottom
.endm
```

Usada para IRQs de hardware (0x20, 0x21). El `IRO_BASE` (0x20) desplaza las IRQs para que no colisionen con excepciones.

### Codigo comun `int_bottom`

```asm
int_bottom:
    pusha                  ; Guardar todos los registros generales
    pushl %ds              ; Guardar segmentos de datos
    pushl %es
    pushl %fs
    pushl %gs

    push %esp              ; Push puntero a la pila (como argumento)
    movl (interruptnumber), %eax
    pushl %eax             ; Push numero de interrupcion
    call handle_interrupt  ; Llamar al manejador en C

    movl %eax, %esp        ; Restaurar pila (handle_interrupt retorna ESP)

    pop %gs                ; Restaurar segmentos
    pop %fs
    pop %es
    pop %ds
    popa                   ; Restaurar registros generales
    add $4, %esp           ; Limpiar error code dummy
    iret                   ; Retornar de interrupcion
```

### Tabla de excepciones

```asm
.global exception_handler_table
exception_handler_table:
    .long handle_interrupt_exception0x00   ; #DE - Divide Error
    .long handle_interrupt_exception0x01   ; #DB - Debug
    .long handle_interrupt_exception0x02   ; #NMI
    .long handle_interrupt_exception0x03   ; #BP - Breakpoint
    .long handle_interrupt_exception0x04   ; #OF - Overflow
    .long handle_interrupt_exception0x05   ; #BR - Bound Range
    .long handle_interrupt_exception0x06   ; #UD - Invalid Opcode
    .long handle_interrupt_exception0x07   ; #NM - Device Not Available
    .long handle_interrupt_exception0x08   ; #DF - Double Fault
    ... (hasta 0x1F)
```

Esta tabla permite iterar y asignar los handlers de excepcion a las entradas de la IDT.

### `ignore_interrupt_request` - Handler por defecto

```asm
ignore_interrupt_request:
    pushl %eax
    movb $0x20, %al
    outb %al, $0x20       ; Enviar EOI al PIC maestro
    popl %eax
    iret
```

Para interrupciones sin handler real, solo envia el **EOI (End of Interrupt)** y retorna.

## interrupts.c - Manejador en C

### `init_interrupt_manager()` - Configuracion de la IDT

```c
void init_interrupt_manager(global_descriptor_table *gdt)
{
    uint16_t code_segment = gdt_get_code_selector(gdt);  // 0x10

    // Rellenar todas las entradas con ignore_interrupt_request
    for (uint16_t i = 0; i < 256; i++)
        set_interrupt_descriptor_table_entry(i, code_segment,
            &ignore_interrupt_request, 0, 0xE);

    // Configurar excepciones 0x00-0x1F
    for (uint16_t i = 0; i < 0x20; i++)
        set_interrupt_descriptor_table_entry(i, code_segment,
            exception_handler_table[i], 0, 0xE);

    // Configurar IRQ 0 (Timer) y IRQ 1 (Teclado)
    set_interrupt_descriptor_table_entry(0x20, code_segment,
        &handle_interrupt_request0x00, 0, 0xE);
    set_interrupt_descriptor_table_entry(0x21, code_segment,
        &handle_interrupt_request0x01, 0, 0xE);

    // Inicializar PICs (remapeo + mascara)
    outb(0x20, 0x11);  outb(0xA0, 0x11);
    outb(0x21, 0x20);  outb(0xA1, 0x28);
    outb(0x21, 0x04);  outb(0xA1, 0x02);
    outb(0x21, 0x01);  outb(0xA1, 0x01);
    outb(0x21, 0xFC);  outb(0xA1, 0xFF);

    // Cargar IDT y habilitar interrupciones
    idt_pointer idt;
    idt.size = (256 * sizeof(gate_descriptor)) - 1;
    idt.base = (uint32_t)interrupt_descriptor_table;
    asm volatile("lidt %0" : : "m"(idt));
    asm volatile("sti");   // Habilitar interrupciones
}
```

### `set_interrupt_descriptor_table_entry()` - Configurar una entrada

```c
void set_interrupt_descriptor_table_entry(uint8_t interrupt_number,
    uint16_t code_segment_selector_offset, void (*handler)(),
    uint8_t descriptor_privilege_level, uint8_t descriptor_type)
{
    const uint8_t IDT_DESC_PRESENT = 0x80;

    idt[interrupt_number].handler_address_low_bits =
        ((uint32_t)handler) & 0xFFFF;
    idt[interrupt_number].handler_address_high_bits =
        ((uint32_t)handler >> 16) & 0xFFFF;
    idt[interrupt_number].gdt_code_segment_selector =
        code_segment_selector_offset;
    idt[interrupt_number].access =
        IDT_DESC_PRESENT | descriptor_type | ((descriptor_privilege_level & 3) << 5);
    idt[interrupt_number].reserved = 0;
}
```

### `handle_interrupt()` - Despacho central

```c
uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t stack_pointer)
{
    // Excepciones: imprimir y detener CPU
    if (interrupt_number < 0x20) {
        serial_write_string(COM1_BASE_ADDRESS, "EXCEPCION ", 10);
        char digit = (char)('0' + interrupt_number);
        serial_write_string(COM1_BASE_ADDRESS, &digit, 1);
        asm volatile("cli; hlt");
    }

    // Despacho a handlers especificos
    if (interrupt_number == 0x21) {
        stack_pointer = keyboard_handler(stack_pointer);
    } else {
        serial_write_string(COM1_BASE_ADDRESS, "INTERRUPT ", 10);
    }

    // Enviar EOI al PIC
    outb(0x20, 0x20);
    if (interrupt_number >= 0x28)
        outb(0xA0, 0x20);

    return stack_pointer;
}
```

**Flujo del despacho:**

```
handle_interrupt(num, esp)
    │
    ├─ Si num < 0x20: Excepcion → imprimir y HLT
    │
    ├─ Si num == 0x21: keyboard_handler(esp)
    │
    ├─ Si num != 0x21: imprimir "INTERRUPT"
    │
    ├─ Enviar EOI (0x20) al PIC maestro
    └─ Si num >= 0x28: enviar EOI al PIC esclavo
```

### EOI - End of Interrupt

Despues de atender una interrupcion, se **debe** enviar un EOI al PIC. Sin esto, el PIC no envia mas interrupciones.

```c
outb(0x20, 0x20);       // EOI al PIC maestro
if (interrupt_number >= 0x28)
    outb(0xA0, 0x20);   // EOI al PIC esclavo (IRQ 8-15)
```

## Flujo completo de una interrupcion

```
1. Hardware genera IRQ 1 (teclado)
2. PIC maestro recibe la IRQ
3. PIC verifica mascara (IRQ 1 habilitada → 0xFC tiene bit 1 = 0)
4. PIC envia INT 0x21 al CPU
5. CPU busca entrada 0x21 en la IDT
6. CPU ejecuta handle_interrupt_request0x01 (stub en ensamblador)
7. Stub guarda registros (pusha), segmentos (push ds/es/fs/gs)
8. Stub llama a handle_interrupt(0x21, esp)
9. handle_interrupt despacha a keyboard_handler(esp)
10. keyboard_handler lee scancode de puerto 0x60
11. keyboard_handler escribe caracter en pantalla
12. handle_interrupt envia EOI al PIC
13. handle_interrupt retorna nuevo ESP
14. Stub restaura registros y segmentos
15. Stub ejecuta IRET
16. CPU reanuda el codigo interrumpido
```

## Navegacion

| Anterior | Siguiente |
|----------|-----------|
| [GDT](gdt.md) | [Driver de teclado](keyboard.md) |

| Relacionados |
|--------------|
| [Loader en ensamblador](loader-ensamblador.md) |
| [Puerto serie](serial.md) |
