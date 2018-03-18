#pragma once

#include "main.h"

#include <stdint.h>

typedef struct {
    uint16_t battery_voltage_mV;
    int16_t battery_current_mA;
    int8_t battery_temperature_dgC;
} Measurements_t;

//! Calculate measured values
//!
//! Do oversampling and conversion to useful values by evaluating the ADC measurements.
void BMS_calculate_measurements (void);
