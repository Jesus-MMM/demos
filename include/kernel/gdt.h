/* gdt.h - Definiciones de la Tabla de Descriptores Globales (GDT) para modo
   protegido i386. Define los descriptores de segmento de codigo y datos
   necesarios para el funcionamiento basico del kernel. */

#pragma once

#include "drivers/serial.h"
#include "types.h"

/** SegmentDescriptor - Descriptor de segmento de 8 bytes (64 bits) formato
 * Intel. Define la base, limite y atributos de un segmento de memoria. */
typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t flags_and_limit_high;
    uint8_t base_high;
} SegmentDescriptor;

/** global_descriptor_table - GDT completa de 4 entradas (32 bytes):
 * descriptor nulo, no usado, segmento de codigo y segmento de datos. */
typedef struct __attribute__((packed)) {
    SegmentDescriptor null_segment_selector;
    SegmentDescriptor unused_segment_selector;
    SegmentDescriptor code_segment_selector;
    SegmentDescriptor data_segment_selector;
} global_descriptor_table;

/* Selectores de segmento (offset dentro de la GDT) */
#define GDT_CODE_SELECTOR 0x10
#define GDT_DATA_SELECTOR 0x18

/** init_gdt - Inicializa los descriptores, carga la GDTR con LGDT y
 * recarga los segmentos CS (far jump) y DS/ES/FS/GS/SS.
 * @gdt: puntero a la estructura global_descriptor_table */
void init_gdt(global_descriptor_table *gdt);

/** init_segment_descriptor - Configura un descriptor de segmento con los
 * valores de base, limite y tipo especificados.
 * @sd:    puntero al descriptor a inicializar
 * @base:  direccion base del segmento
 * @limit: limite del segmento (en bytes o paginas segun granularidad)
 * @type:  byte de acceso/tipo (p. ej. 0x9A para codigo, 0x92 para datos) */
void init_segment_descriptor(SegmentDescriptor *sd, uint32_t base, uint32_t limit, uint8_t type);

/** segment_descriptor_get_base - Obtiene la base de 32 bits desde un
 * descriptor de segmento.
 * @sd: puntero al descriptor
 * Retorna: direccion base completa */
uint32_t segment_descriptor_get_base(SegmentDescriptor *sd);

/** segment_descriptor_get_limit - Obtiene el limite de 20 bits desde un
 * descriptor de segmento, ajustando por granularidad de pagina si es necesario.
 * @sd: puntero al descriptor
 * Retorna: limite completo en bytes */
uint32_t segment_descriptor_get_limit(SegmentDescriptor *sd);

/** gdt_get_code_selector - Calcula el offset (selector) del descriptor de
 * codigo dentro de la GDT.
 * @gdt: puntero a la GDT
 * Retorna: selector de 16 bits (0x10) */
uint16_t gdt_get_code_selector(global_descriptor_table *gdt);

/** gdt_get_data_selector - Calcula el offset (selector) del descriptor de
 * datos dentro de la GDT.
 * @gdt: puntero a la GDT
 * Retorna: selector de 16 bits (0x18) */
uint16_t gdt_get_data_selector(global_descriptor_table *gdt);
