#pragma once

#include <stdint.h>

#include "mcc_generated_files/coretimer.h"

typedef unsigned int uint;

const uint16_t REFERENCE_VOLTAGE_mV = 2500;

inline void delay_ms (uint16_t ms)
{
    uint32_t start_tick = tick_ms;
    
    while (tick_ms - start_tick < ms);
}

inline uint32_t * va_2_pa (uint32_t * virtual_address) {
    uint32_t * physical_address = 0;
    
    return physical_address;
}

void Error_Handler (void);