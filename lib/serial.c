/* serial.c - Implementacion del controlador de puerto serie (UART 16550)
   para comunicacion de depuracion en modo texto. */

#include "serial.h"

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
