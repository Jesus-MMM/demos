/* pci.h - Gestor de dispositivos PCI.
   Proporciona funciones para leer/escribir registros de configuracion
   PCI a traves de los puertos 0xCF8 (direccion) y 0xCFC (datos),
   enumerar dispositivos conectados al bus y obtener descriptores
   con vendor_id, device_id, clase, subclase e interrupcion. */

#pragma once

#include "asm.h"
#include "drivers/driver.h"
#include "types.h"

enum base_address_register_type { memory_mapping = 0, input_output = 1 };

typedef struct {

    bool prefetchable;
    uint8_t *address;
    uint32_t size;
    enum base_address_register_type type;

} base_address_register;

/** pci_device_descriptor - Descriptor de un dispositivo PCI.
   Contiene la informacion basica de identificacion y configuracion
   de un dispositivo leido del bus PCI. */
typedef struct {
    uint32_t port_base;
    uint32_t interrupt;

    uint16_t bus;
    uint16_t device;
    uint16_t funtion;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t sub_class_id;
    uint8_t interface_id;
    uint8_t revision;

} pci_device_descriptor;

/** pci_read - Lee un registro de configuracion de un dispositivo PCI.
   Envia la direccion compuesta (bus/device/function/offset) al puerto 0xCF8
   y lee la respuesta de 32 bits del puerto 0xCFC, desplazando segun el
   offset para extraer el campo solicitado.
   @bus:             numero de bus (0-255)
   @device:          numero de dispositivo en el bus (0-31)
   @funtion:         numero de funcion del dispositivo (0-7)
   @register_offset: offset del registro de configuracion (0-255)
   Return:           valor leido del registro */
uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t funtion, uint32_t register_offset);

/** pci_write - Escribe un valor en un registro de configuracion PCI.
   @bus:             numero de bus (0-255)
   @device:          numero de dispositivo en el bus (0-31)
   @funtion:         numero de funcion del dispositivo (0-7)
   @register_offset: offset del registro de configuracion (0-255)
   @value:           valor de 32 bits a escribir
   Return:           0 en caso de exito */
uint32_t pci_write(uint16_t bus, uint16_t device, uint16_t funtion,
                   uint32_t register_offset, // NOLINT(bugprone-easily-swappable-parameters)
                   uint32_t value);

/** device_has_functions - Verifica si un dispositivo PCI tiene multiples funciones.
   Lee el byte de Header Type (registro 0x0E) y comprueba el bit 7
   que indica si el dispositivo es multifuncion.
   @bus:    numero de bus (0-255)
   @device: numero de dispositivo en el bus (0-31)
   Return:  true si tiene multiples funciones, false si es single-function */
bool device_has_functions(uint32_t bus, uint16_t device);

/** select_drivers - Escanea el bus PCI y registra drivers para dispositivos encontrados.
   Recorre todos los buses (0-255) y dispositivos (0-31), verificando vendor_id
   para detectar dispositivos presentes. Para cada uno obtiene su descriptor
   e imprime informacion de depuracion por el puerto serie.
   @manager: puntero al administrador de drivers donde registrar los dispositivos */
void select_drivers(driver_manager_t *manager);

/** get_device_descriptor - Obtiene el descriptor completo de un dispositivo PCI.
   Lee los registros de configuracion del dispositivo para completar
   todos los campos del pci_device_descriptor: vendor_id, device_id,
   class_id, sub_class_id, interface_id, revision e interrupt.
   @bus:    numero de bus (0-255)
   @device: numero de dispositivo en el bus (0-31)
   @funtion: numero de funcion del dispositivo (0-7)
   @out:    puntero al descriptor a rellenar */
void get_device_descriptor(uint16_t bus, uint16_t device, uint16_t funtion,
                           pci_device_descriptor *out);

void get_address_register(uint16_t bus, uint16_t device, uint16_t funtion, uint16_t bar,
                          base_address_register *out);
                          
/** get_driver - Identifica el dispositivo PCI y retorna un driver compatible.
   Lee vendor_id y class_id para reconocer dispositivos conocidos (p.ej.
   AMD am79c973, VGA) e imprime informacion de depuracion.
   @device: descriptor del dispositivo PCI a identificar
   Return: puntero al driver o NULL si no se reconoce el dispositivo */
driver_t *get_driver(pci_device_descriptor *device);
