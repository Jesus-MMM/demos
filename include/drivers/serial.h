/* serial.h - Controlador de puerto serie (UART 16550).
   Proporciona funciones para inicializar, leer y escribir sobre
   el puerto serie para propositos de depuracion. */

#pragma once

#include "asm.h"
#include "types.h"

/* Direcciones base de los puertos serie estandar */
#define COM1_BASE_ADDRESS 0x3F8
#define COM2_BASE_ADDRESS 0x2F8

/* Macros de acceso a los registros del UART relativo a la base */
#define SERIAL_DATA_PORT(base_address) ((base_address))
#define SERIAL_INTERRUPT_PORT(base_address) ((base_address) + 1)
#define SERIAL_FIFO_CMD_PORT(base_address) ((base_address) + 2)
#define SERIAL_LINE_CMD_PORT(base_address) ((base_address) + 3)
#define SERIAL_LINE_STATUS_PORT(base_address) ((base_address) + 5)

static const char hex_chars[] = "0123456789ABCDEF";

/** serial_init - Inicializa el puerto serie en 8N1 con divisor de baudios 1.
 * @com: direccion base del puerto serie (COM1_BASE_ADDRESS o COM2_BASE_ADDRESS) */
void serial_init(uint16_t com);

/** serial_write - Escribe un byte en el puerto serie.
 * @com: direccion base del puerto serie
 * @data: byte a transmitir */
void serial_write(uint16_t com, uint8_t data);

/** serial_write_hex16 - Escribe un valor de 16 bits en formato hexadecimal.
   @com:   direccion base del puerto serie
   @value: valor de 16 bits a imprimir (ej. 0xABCD) */
void serial_write_hex16(uint16_t com, uint16_t value);

/** serial_write_u8 - Escribe un valor de 8 bits en formato decimal.
   @com:   direccion base del puerto serie
   @value: valor de 8 bits a imprimir (0-255) */
void serial_write_u8(uint16_t com, uint8_t value);

/** serial_write_string - Escribe una cadena en el puerto serie.
 * @com: direccion base del puerto serie
 * @buffer: puntero a los datos a transmitir
 * @len: cantidad de bytes a transmitir */
void serial_write_string(uint16_t com, const char *buffer, uint32_t len);

/** serial_read - Lee un byte del puerto serie (bloqueante).
 * @com: direccion base del puerto serie
 * Return: caracter recibido */
char serial_read(uint16_t com);

/** is_transmition_buffer_empty - Verifica si el buffer de transmision esta vacio.
 * @com: direccion base del puerto serie
 * Return: 1 si vacio, 0 si ocupado */
int8_t is_transmition_buffer_empty(uint16_t com);

/** is_data_received - Verifica si hay datos disponibles para lectura.
 * @com: direccion base del puerto serie
 * Return: 1 si hay datos, 0 si no */
int8_t is_data_received(uint16_t com);
