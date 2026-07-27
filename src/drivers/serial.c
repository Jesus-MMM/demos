/* serial.c - Implementacion del controlador de puerto serie (UART 16550)
   para comunicacion de depuracion en modo texto. */

#include "drivers/serial.h"

void serial_init(uint16_t com)
{
    // desactivar interrupciones
    outb(SERIAL_INTERRUPT_PORT(com), 0x00);

    // establecer el baudrate
    uint16_t divisor = 1;
    outb(SERIAL_LINE_CMD_PORT(com), 0x80); // DLAB de 1
    outb(SERIAL_DATA_PORT(com),
         divisor & 0x00FF); // enviar el byte menos signficativo de la division
    outb(SERIAL_DATA_PORT(com) + 1,
         (divisor >> 8) & 0x00FF); // enviar el byte mas significativo de la division

    // configurar LINE 8N1
    outb(SERIAL_LINE_CMD_PORT(com), 0x03);

    // configurar la cola fifo
    outb(SERIAL_FIFO_CMD_PORT(com), 0xC7);
}

void serial_write(uint16_t com, uint8_t data)
{
    while (!is_transmition_buffer_empty(com)) {
    }
    outb(SERIAL_DATA_PORT(com), data);
}

void serial_write_string(uint16_t com, const char *buffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        serial_write(com, buffer[i]);
    }
}

void serial_write_hex16(uint16_t com, uint16_t value)
{
    serial_write(com, hex_chars[(value >> 12) & 0xF]);
    serial_write(com, hex_chars[(value >> 8) & 0xF]);
    serial_write(com, hex_chars[(value >> 4) & 0xF]);
    serial_write(com, hex_chars[value & 0xF]);
}

void serial_write_u8(uint16_t com, uint8_t value)
{
    if (value >= 100) {
        serial_write(com, '0' + (value / 100));
    }
    if (value >= 10) {
        serial_write(com, '0' + ((value / 10) % 10));
    }
    serial_write(com, '0' + (value % 10));
}

char serial_read(uint16_t com)
{
    while (!is_data_received(com)) {
    }
    return (char)inb(SERIAL_DATA_PORT(com));
}

int8_t is_transmition_buffer_empty(uint16_t com)
{
    return (int8_t)((inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20) == 0x20);
}

int8_t is_data_received(uint16_t com)
{
    return (int8_t)((inb(SERIAL_LINE_STATUS_PORT(com)) & 0x01) == 0x01);
}
