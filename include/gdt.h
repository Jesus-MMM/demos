#pragma once

#include "types.h"

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t flags_and_limit_high;
    uint8_t base_high;
} SegmentDescriptor;

typedef struct __attribute__((packed)) {
    SegmentDescriptor null_segment_selector;
    SegmentDescriptor unused_segment_selector;
    SegmentDescriptor code_segment_selector;
    SegmentDescriptor data_segment_selector;
} global_descriptor_table;

#define GDT_CODE_SELECTOR 0x10
#define GDT_DATA_SELECTOR 0x18

void init_gdt(global_descriptor_table *gdt);
void init_segment_descriptor(SegmentDescriptor *sd, uint32_t base, uint32_t limit, uint8_t type);

uint32_t segment_descriptor_get_base(SegmentDescriptor *sd);
uint32_t segment_descriptor_get_limit(SegmentDescriptor *sd);

uint16_t gdt_get_code_selector(global_descriptor_table *gdt);
uint16_t gdt_get_data_selector(global_descriptor_table *gdt);
