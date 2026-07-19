#pragma once

#include "types.h"
#include "asm.h"
#include "drivers/vga.h"
#include "drivers/serial.h"

void mouse_init(void);
uint32_t mouse_handler(uint32_t esp);