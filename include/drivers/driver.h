/* driver.h - Interfaz base para drivers del kernel.
   Define un tipo generico driver_t con punteros a funciones virtuales
   (activate, reset, deactivate, handle_interrupt) y un driver_manager_t
   que registra y activa todos los drivers del sistema. */

#pragma once

#include "types.h"
#include "util/util_lib.h"

#define MAX_DRIVERS 256

/** Punteros a funciones de callback para eventos de entrada.
   El parametro @data permite pasar contexto (p.ej. el driver que genera el evento). */
typedef void (*on_key_down_fn)(char c, void *data);
typedef void (*on_key_up_fn)(char c, void *data);
typedef void (*on_mouse_move_fn)(int8_t x_offset, int8_t y_offset, void *data);
typedef void (*on_mouse_button_fn)(uint8_t button, int8_t x, int8_t y, bool pressed, void *data);

/** driver_t - Estructura base que representa un driver del kernel.
   Cada driver concreto (teclado, mouse, etc.) contiene una de estas
   estructuras y asigna sus punteros a funciones. */
typedef struct driver {
    const char *name;
    uint8_t irq; /* Numero de IRQ que maneja (0 = sin IRQ) */

    void (*activate)(struct driver *self);
    int (*reset)(struct driver *self);
    void (*deactivate)(struct driver *self);
    uint32_t (*handle_interrupt)(struct driver *self, uint32_t esp);
} driver_t;

/** driver_manager_t - Administrador centralizado de drivers.
   Mantiene un arreglo de punteros a drivers y provee operaciones
   para registrarlos y activarlos en orden. */
typedef struct {
    driver_t *drivers[MAX_DRIVERS];
    uint32_t num_drivers;
} driver_manager_t;

/** driver_manager_init - Inicializa el administrador de drivers.
   @manager: puntero al administrador a inicializar */
void driver_manager_init(driver_manager_t *manager);

/** driver_manager_add - Registra un driver en el administrador.
   @manager: puntero al administrador
   @driver:  puntero al driver a registrar */
void driver_manager_add(driver_manager_t *manager, driver_t *driver);

/** driver_manager_activate_all - Activa todos los drivers registrados
   invocando su funcion activate() en el orden en que fueron agregados.
   @manager: puntero al administrador */
void driver_manager_activate_all(driver_manager_t *manager);

/** driver_manager_get_driver_for_irq - Busca el driver registrado que
   maneja el IRQ indicado.
   @manager: puntero al administrador
   @irq:     numero de IRQ a buscar
   Return:   puntero al driver, o NULL si ninguno lo maneja */
driver_t *driver_manager_get_driver_for_irq(driver_manager_t *manager, uint8_t irq);
