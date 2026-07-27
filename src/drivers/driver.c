/* driver.c - Implementacion del administrador de drivers del kernel.
   Proporciona la estructura base para registrar, activar y despachar
   interrupciones a traves de un arreglo centralizado de drivers. */

#include "drivers/driver.h"
#include "drivers/serial.h"

void driver_manager_init(driver_manager_t *manager)
{
    manager->num_drivers = 0;
    for (uint32_t i = 0; i < MAX_DRIVERS; i++) {
        manager->drivers[i] = NULL;
    }
}

void driver_manager_add(driver_manager_t *manager, driver_t *driver)
{
    if (manager->num_drivers >= MAX_DRIVERS) {
        serial_write_string(COM1_BASE_ADDRESS, "[DRVMGR] Driver table full\n", 26);
        return;
    }
    manager->drivers[manager->num_drivers] = driver;
    manager->num_drivers++;

    serial_write_string(COM1_BASE_ADDRESS, "[DRVMGR] Added driver: ", 23);
    serial_write_string(COM1_BASE_ADDRESS, driver->name, strlen(driver->name));

    const char nl[] = "\n";
    serial_write_string(COM1_BASE_ADDRESS, nl, 1);
}

void driver_manager_activate_all(driver_manager_t *manager)
{
    for (uint32_t i = 0; i < manager->num_drivers; i++) {
        driver_t *drv = manager->drivers[i];
        if (drv && drv->activate) {
            drv->activate(drv);
        }
    }
}

driver_t *driver_manager_get_driver_for_irq(driver_manager_t *manager, uint8_t irq)
{
    for (uint32_t i = 0; i < manager->num_drivers; i++) {
        driver_t *drv = manager->drivers[i];
        if (drv && drv->irq == irq) {
            return drv;
        }
    }
    return NULL;
}
