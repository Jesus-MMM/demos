/* interrupts.h - Gestion de interrupciones y excepciones del CPU i386.
   Define la Tabla de Descriptores de Interrupcion (IDT), la inicializacion
   del PIC 8259A y los manejadores de interrupciones y excepciones. */

#pragma once

#include "gdt.h"
#include "serial.h"
#include "types.h"

/** gate_descriptor - Entrada de la IDT de 8 bytes (formato Intel).
 * Describe un manejador de interrupcion o excepcion: direccion del handler,
 * selector de segmento de codigo y atributos de acceso. */
typedef struct __attribute__((packed)) {
    uint16_t handler_address_low_bits;
    uint16_t gdt_code_segment_selector;
    uint8_t reserved;
    uint8_t access;
    uint16_t handler_address_high_bits;
} gate_descriptor;

/** Tabla global de descriptores de interrupcion (256 entradas). */
extern gate_descriptor interrupt_descriptor_table[256];

/** init_interrupt_manager - Inicializa la IDT (establece manejadores para
 * excepciones 0x00-0x1F e IRQs 0x20-0x21), remapea el PIC 8259A maestro/esclavo,
 * carga la IDTR con LIDT y habilita interrupciones (STI).
 * @gdt: puntero a la GDT (para obtener el selector de segmento de codigo) */
void init_interrupt_manager(global_descriptor_table *gdt);

/** handle_interrupt - Manejador comun en C para interrupciones y excepciones.
 * Para excepciones (num < 0x20) imprime el numero y detiene la CPU (HLT).
 * Para IRQs envia EOI al PIC correspondiente.
 * @interrupt_number: numero del vector de interrupcion
 * @stack_pointer:    puntero a la pila al momento de la interrupcion
 * Retorna: el puntero de pila (potencialmente modificado) */
uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t stack_pointer);

/** ignore_interrupt_request - Manejador minimo que solo envia EOI al PIC
 * maestro y retorna. Usado como fallback para vectores sin manejador real. */
void ignore_interrupt_request();

/** set_interrupt_descriptor_table_entry - Configura una entrada individual
 * de la IDT con la direccion del handler, selector y atributos indicados.
 * @interrupt_number:           vector a configurar (0-255)
 * @code_segment_selector_offset: selector GDT de codigo
 * @handler:                    funcion manejadora
 * @descriptor_privilege_level: nivel de privilegio (0-3)
 * @descriptor_type:            tipo de compuerta (0xE = interrupt gate) */
void set_interrupt_descriptor_table_entry(uint8_t interrupt_number,
                                          uint16_t code_segment_selector_offset, void (*handler)(),
                                          uint8_t descriptor_privilege_level,
                                          uint8_t descriptor_type);
