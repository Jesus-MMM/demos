#include "kernel/gdt.h"
#include "drivers/serial.h"
#include "util/util_lib.h"

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} gdtr_t;

void init_gdt(global_descriptor_table *gdt)
{
    init_segment_descriptor(&gdt->null_segment_selector, 0, 0, 0);
    init_segment_descriptor(&gdt->unused_segment_selector, 0, 0, 0);
    init_segment_descriptor(&gdt->code_segment_selector, 0, 0xFFFFFFFFU, 0x9A);
    init_segment_descriptor(&gdt->data_segment_selector, 0, 0xFFFFFFFFU, 0x92);

    serial_write_string(COM1_BASE_ADDRESS, "[GDT] Descriptors filled\n", 25);

    gdtr_t gdtr;
    gdtr.base = (uint32_t)gdt;
    gdtr.limit = sizeof(global_descriptor_table) - 1;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));

    serial_write_string(COM1_BASE_ADDRESS, "[GDT] GDTR loaded\n", 18);

    /* far jump para recargar CS con el nuevo code segment descriptor */
    __asm__ volatile(
        "ljmp %0, $1f\n"
        "1:\n"
        : : "i"(GDT_CODE_SELECTOR)
    );

    serial_write_string(COM1_BASE_ADDRESS, "[GDT] CS reloaded\n", 18);

    /* recargar data segment registers */
    __asm__ volatile(
        "mov %0, %%ds\n"
        "mov %0, %%es\n"
        "mov %0, %%fs\n"
        "mov %0, %%gs\n"
        "mov %0, %%ss\n"
        : : "r"((uint16_t)GDT_DATA_SELECTOR)
    );

    serial_write_string(COM1_BASE_ADDRESS, "[GDT] DS/ES/FS/GS/SS reloaded\n", 30);
    serial_write_string(COM1_BASE_ADDRESS, "[GDT] Ready\n", 12);
}

void init_segment_descriptor(SegmentDescriptor *sd, uint32_t base, // NOLINT(bugprone-easily-swappable-parameters)
                             uint32_t limit, // NOLINT(bugprone-easily-swappable-parameters)
                             uint8_t type) // NOLINT(bugprone-easily-swappable-parameters)
{
    if (limit <= 65536) {
        sd->flags_and_limit_high = 0x40;
    } else {
        if ((limit & 0xFFF) != 0xFFF) {
            limit = (limit >> 12) - 1;
        } else {
            limit = limit >> 12;
        }

        sd->flags_and_limit_high = 0xC0;
    }

    sd->limit_low = limit & 0xFFFF;
    sd->flags_and_limit_high |= (limit >> 16) & 0xF;

    sd->base_low = base & 0xFFFF;
    sd->base_mid = (base >> 16) & 0xFF;
    sd->base_high = (base >> 24) & 0xFF;

    sd->access_byte = type;
}

uint32_t segment_descriptor_get_base(SegmentDescriptor *sd)
{
    return ((uint32_t)sd->base_high << 24)
         | ((uint32_t)sd->base_mid << 16)
         | sd->base_low;
}

uint32_t segment_descriptor_get_limit(SegmentDescriptor *sd)
{
    uint32_t limit = ((uint32_t)(sd->flags_and_limit_high & 0xF) << 16)
                   | sd->limit_low;

    if ((sd->flags_and_limit_high & 0xC0) == 0xC0) {
        limit = (limit << 12) | 0xFFF;
    }

    return limit;
}

uint16_t gdt_get_code_selector(global_descriptor_table *gdt)
{
    return (uint8_t *)&gdt->code_segment_selector - (uint8_t *)gdt;
}

uint16_t gdt_get_data_selector(global_descriptor_table *gdt)
{
    return (uint8_t *)&gdt->data_segment_selector - (uint8_t *)gdt;
}
