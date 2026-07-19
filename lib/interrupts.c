#include "interrupts.h"
#include "asm.h"
#include "keyboard.h"

gate_descriptor interrupt_descriptor_table[256];

extern void handle_interrupt_request0x00(void);
extern void handle_interrupt_request0x01(void);
extern void (*exception_handler_table[])();

typedef struct __attribute__((packed)) {
    uint16_t size;
    uint32_t base;
} idt_pointer;

void set_interrupt_descriptor_table_entry(uint8_t interrupt_number,
                                          uint16_t code_segment_selector_offset, void (*handler)(),
                                          uint8_t descriptor_privilege_level,
                                          uint8_t descriptor_type)
{

    const uint8_t IDT_DESC_PRESENT = 0x80;

    interrupt_descriptor_table[interrupt_number].handler_address_low_bits =
        ((uint32_t)handler) & 0xFFFF;

    interrupt_descriptor_table[interrupt_number].handler_address_high_bits =
        ((uint32_t)handler >> 16) & 0xFFFF;

    interrupt_descriptor_table[interrupt_number].gdt_code_segment_selector =
        code_segment_selector_offset;

    interrupt_descriptor_table[interrupt_number].access =
        IDT_DESC_PRESENT | descriptor_type | ((descriptor_privilege_level & 3) << 5);

    interrupt_descriptor_table[interrupt_number].reserved = 0;
}

void init_interrupt_manager(global_descriptor_table *gdt)
{
    uint16_t code_segment = gdt_get_code_selector(gdt);
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for (uint16_t i = 0; i < 256; i++) {
        set_interrupt_descriptor_table_entry(i, code_segment, &ignore_interrupt_request, 0,
                                             IDT_INTERRUPT_GATE);
    }

    for (uint16_t i = 0; i < 0x20; i++) {
        set_interrupt_descriptor_table_entry(i, code_segment, exception_handler_table[i], 0,
                                             IDT_INTERRUPT_GATE);
    }

    set_interrupt_descriptor_table_entry(0x20, code_segment, &handle_interrupt_request0x00, 0,
                                         IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x21, code_segment, &handle_interrupt_request0x01, 0,
                                         IDT_INTERRUPT_GATE);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    idt_pointer idt;
    idt.size = (256 * sizeof(gate_descriptor)) - 1;
    idt.base = (uint32_t)interrupt_descriptor_table;

    asm volatile("lidt %0" : : "m"(idt));
    asm volatile("sti");
}

uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t stack_pointer) // NOLINT(bugprone-easily-swappable-parameters)
{

    if (interrupt_number != 0x20) {
        serial_write_string(COM1_BASE_ADDRESS, "INTERRUPT ", 10);
        char digit = (char)('0' + interrupt_number);
        serial_write_string(COM1_BASE_ADDRESS, &digit, 1);
    }

    if (interrupt_number < 0x20) {
        serial_write_string(COM1_BASE_ADDRESS, "EXCEPCION ", 10);
        char digit = (char)('0' + interrupt_number);
        serial_write_string(COM1_BASE_ADDRESS, &digit, 1);
        serial_write_string(COM1_BASE_ADDRESS, " - HALT\n", 8);
        asm volatile("cli; hlt");
    }

    if (interrupt_number == 0x21) {
        stack_pointer = keyboard_handler(stack_pointer);
    }

    outb(0x20, 0x20);
    if (interrupt_number >= 0x28) {
        outb(0xA0, 0x20);
    }

    return stack_pointer;
}
