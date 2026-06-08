#include "timer.h"

void delay(uint32_t iterations)
{
    for (volatile uint32_t i = 0; i < iterations; i++);
}
