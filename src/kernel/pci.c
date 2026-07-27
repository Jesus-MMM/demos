#include "kernel/pci.h"
#include "drivers/serial.h"

#define PCI_DATA_PORT 0xCFC
#define PCI_COMMAND_PORT 0xCF8

uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t funtion, uint32_t register_offset)
{
    uint32_t id = (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) |
                  ((funtion & 0x07) << 8) | (register_offset & 0xFC);

    outl(PCI_COMMAND_PORT, id);

    uint32_t result = inl(PCI_DATA_PORT);

    return result >> (8 * (register_offset % 4));
}

uint32_t pci_write(uint16_t bus, uint16_t device, uint16_t funtion, uint32_t register_offset, // NOLINT(bugprone-easily-swappable-parameters)
                   uint32_t value)
{
    uint32_t id = (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) |
                  ((funtion & 0x07) << 8) | (register_offset & 0xFC);

    outl(PCI_COMMAND_PORT, id);
    outl(PCI_DATA_PORT, value);

    return 0;
}

uint8_t device_has_functions(uint32_t bus, uint16_t device)
{
    return pci_read(bus, device, 0, 0x0E) & (1 << 7);
}

void select_drivers(driver_manager_t *manager)
{
    (void)manager;
    serial_write_string(COM1_BASE_ADDRESS, "[PCI] scan start\n", 17);

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t device = 0; device < 32; device++) {
            uint16_t vid = (uint16_t)pci_read(bus, device, 0, 0x00);
            if (vid == 0x0000 || vid == 0xFFFF) {
                continue;
            }

            int num_functions = device_has_functions(bus, device) ? 8 : 1;
            for (int funtion = 0; funtion < num_functions; funtion++) {
                pci_device_descriptor pci_device;
                get_device_descriptor(bus, device, funtion, &pci_device);

                if (pci_device.vendor_id == 0x0000 || pci_device.vendor_id == 0xFFFF) {
                    break;
                }

                serial_write_string(COM1_BASE_ADDRESS, "[PCI] B:", 8);
                serial_write_u8(COM1_BASE_ADDRESS, bus);
                serial_write_string(COM1_BASE_ADDRESS, " D:", 3);
                serial_write_u8(COM1_BASE_ADDRESS, device);
                serial_write_string(COM1_BASE_ADDRESS, " F:", 3);
                serial_write_u8(COM1_BASE_ADDRESS, funtion);
                serial_write_string(COM1_BASE_ADDRESS, " V:", 3);
                serial_write_hex16(COM1_BASE_ADDRESS, pci_device.vendor_id);
                serial_write_string(COM1_BASE_ADDRESS, " DEV:", 5);
                serial_write_hex16(COM1_BASE_ADDRESS, pci_device.device_id);
                serial_write(COM1_BASE_ADDRESS, '\n');
            }
        }
    }
    serial_write_string(COM1_BASE_ADDRESS, "[PCI] scan done\n", 16);
}

void get_device_descriptor(uint16_t bus, uint16_t device, uint16_t funtion,
                           pci_device_descriptor *out)
{
    out->bus = bus;
    out->device = device;
    out->funtion = funtion;

    out->vendor_id = pci_read(bus, device, funtion, 0x00);
    out->device_id = pci_read(bus, device, funtion, 0x02);

    out->class_id = pci_read(bus, device, funtion, 0x0b);
    out->sub_class_id = pci_read(bus, device, funtion, 0x0a);
    out->interface_id = pci_read(bus, device, funtion, 0x09);

    out->revision = pci_read(bus, device, funtion, 0x08);
    out->interrupt = pci_read(bus, device, funtion, 0x3c);
}