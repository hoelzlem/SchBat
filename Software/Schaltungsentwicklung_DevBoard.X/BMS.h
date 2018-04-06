#pragma once

#include "main.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t min_battery_voltage_mV;
    uint16_t max_battery_voltage_mV;
    uint16_t end_of_charge_voltage_mV;
    uint16_t max_battery_charge_current_mA;
    uint16_t max_battery_discharge_current_mA;
    int8_t min_battery_temperature_dgC;
    uint8_t max_battery_temperature_dgC;
    uint32_t cell_capacity_mAs;
} BMS_Configuration_t;

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

//! Update measurements
//!
//! Do oversampling and conversion to measured values (e.g. battery voltage or battery current).
static void update_measurements(void);

static void print_ADC_input_voltages (void);

static void control_led (bool enable_led);

static void control_charger (bool enable_charging);