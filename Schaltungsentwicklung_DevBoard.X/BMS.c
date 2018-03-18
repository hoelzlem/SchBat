// TODO
// * Regelung für Ladestrom und -spannung
// * FSM für BMS bauen mit Logik, wann die Zelle getrennt wird

#include "BMS.h"

#include <mcc_generated_files/adc1.h>

BMS_Configuration_t BMS_configuration;

void BMS_init (void)
{
    // Nothing to do here yet
    return;
}

void BMS_step (void)
{
    BMS_State_t current_state = discharging;
    static BMS_State_t next_state = discharging;

    Measurements_t measurements;

    bool enable_charging = false;

    // Calculate sensor values
    // Ubat = Umeas * Gain = Umeas * 1.8
    measurements.battery_voltage_mV = ( (uint32_t)(ADC1_measured_voltage_mV[2]) * 18) / 10;
    // Ibat = Umeas * Gain + Offset = Umeas * 0.8772 mA/mV - 600 mA
    measurements.battery_current_mA = ((int32_t)(ADC1_measured_voltage_mV[0]) * 8772) / 10000 - 600;
    // Tbat = Umeas * Gain + Offset = Umeas * 0.04564 K/mV - 50 °C
    measurements.battery_temperature_dgC = ((int32_t)(ADC1_measured_voltage_mV[1]) * 4564) / 100000 - 50;

    // BMS FSM
    // Update current state
    current_state = next_state;

    switch (current_state) {
        case charging: {

        } break;
        case charged: {

        } break;
        case discharging: {

        } break;
        case discharged: {

        } break;
    }

    // Funktion implementieren
    // voltage limit und current limit sind abhängig von BMS Konfiguration und derating wegen Temperatur
    // control_charging(enable_charging, voltage_limit, current_limit);


    // Check sensor values against defined limits
    // voltage_limits_violated = measurements.battery_voltage_mV < BMS_configuration.min_battery_voltage_mV || measurements.battery_voltage_mV > BMS_configuration.max_battery_voltage_mV;
    // current_limits_violated = measurements.battery_current_mA < BMS_configuration.min_battery_current_mA || measurements.battery_current_mA > BMS_configuration.max_battery_current_mA;
    // temperature_limits_violated = measurements.battery_temperature_dgC < BMS_configuration.min_battery_temperature_dgC || measurements.battery_temperature_dgC > BMS_configuration.max_battery_temperature_dgC;
}
