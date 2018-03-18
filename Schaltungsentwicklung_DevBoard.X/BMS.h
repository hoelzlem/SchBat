#pragma once

#include "main.h"

#include <stdint.h>

typedef struct {
    uint16_t battery_voltage_mV;
    int16_t battery_current_mA;
    int8_t battery_temperature_dgC;
} Measurements_t;

typedef struct {
    uint16_t min_battery_voltage_mV;
    uint16_t max_battery_voltage_mV;
    uint16_t max_battery_charge_current_mA;
    uint16_t max_battery_discharge_current_mA;
    int8_t min_battery_temperature_dgC;
    uint8_t max_battery_temperature_dgC;
} BMS_Configuration_t;

typedef enum {
    charging,
    charged,
    discharging,
    discharged
} BMS_State_t;

extern BMS_Configuration_t BMS_configuration;

//! Initialise BMS module
//!
//! \pre
//!     BMS_configuration is set
void BMS_init (void);

//! Perform one BMS cycle
//!
//! Must be executed with 1 ms period
//! Check sensor values against limits and act accordingly
void BMS_step (void);
