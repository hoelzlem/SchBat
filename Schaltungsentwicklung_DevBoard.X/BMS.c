// TODO
// * Regelung f√ºr Ladestrom und -spannung
// * FSM f√ºr BMS bauen mit Logik, wann die Zelle getrennt wird

#include "BMS.h"

#include <stdio.h>

#include "mcc_generated_files/adc1.h"

BMS_Configuration_t BMS_configuration;
Measurements_t measurements;

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
    // voltage limit und current limit sind abh√§ngig von BMS Konfiguration und derating wegen Temperatur
    // control_charging(enable_charging, voltage_limit, current_limit);


    // Check sensor values against defined limits
    // voltage_limits_violated = measurements.battery_voltage_mV < BMS_configuration.min_battery_voltage_mV || measurements.battery_voltage_mV > BMS_configuration.max_battery_voltage_mV;
    // current_limits_violated = measurements.battery_current_mA < BMS_configuration.min_battery_current_mA || measurements.battery_current_mA > BMS_configuration.max_battery_current_mA;
    // temperature_limits_violated = measurements.battery_temperature_dgC < BMS_configuration.min_battery_temperature_dgC || measurements.battery_temperature_dgC > BMS_configuration.max_battery_temperature_dgC;
}

void update_measurements (void)
{       
    struct {
        uint16_t I_MEAS;
        uint16_t T_MEAS;
        uint16_t U_MEAS;
        uint16_t EXT_AD_1;
        uint16_t EXT_AD_2;
    } oversampled_values = {0};
    
    uint it_sample;
    
    // Oversample ADC values
    //
    // Accumulate samples
    // 16 samples per channel
    // -> ENOB = ENOB_ADC + bits_oversampling = 9 + ld(16)/2 = 9 + 2 = 11
    for (it_sample = 0; it_sample < 16; it_sample += 1) {
        oversampled_values.I_MEAS += ADC_sample_buffer.I_MEAS[it_sample];
        oversampled_values.T_MEAS += ADC_sample_buffer.T_MEAS[it_sample];
        oversampled_values.U_MEAS += ADC_sample_buffer.U_MEAS[it_sample];
        oversampled_values.EXT_AD_1 += ADC_sample_buffer.EXT_AD_1[it_sample];
        oversampled_values.EXT_AD_2 += ADC_sample_buffer.EXT_AD_2[it_sample];
    }
    
    // Shift for correct number of effective bits
    //
    // Accumulator has bits_ADC + bits_accumulation = 10 + ld(16) = 14
    // -> shift by 3 to get 11 valid bits
    oversampled_values.I_MEAS >>= 3;
    oversampled_values.T_MEAS >>= 3;
    oversampled_values.U_MEAS >>= 3;
    oversampled_values.EXT_AD_1 >>= 3;
    oversampled_values.EXT_AD_2 >>= 3;
    
    // Convert ADC code to measurement
    //
    // Battery Voltage
    //
    // Gain = 2.198 mV
    // Offset = 0 A
    measurements.battery_voltage_mV = (oversampled_values.U_MEAS * 2198) / 1000;
    
    // Battery Current
    //
    // Gain = 1.068 mA
    // Offset = 0 A -> 1422
    measurements.battery_current_mA = ( ( (int32_t)oversampled_values.I_MEAS - 1422) * 1068) / 1000;
    
    // Battery Temperature
    //
    // Gain = 56 m∞C
    // Offset = 0∞C -> 897
    measurements.battery_temperature_dgC = ( ( (int32_t)oversampled_values.T_MEAS - 897) * 56) / 1000;
    
    // External ADCs
    //
    // Gain = 1
    // Offset = 0
    measurements.EXT_ADC_1_mV = (oversampled_values.EXT_AD_1 * REFERENCE_VOLTAGE_mV) >> 11;
    measurements.EXT_ADC_2_mV = (oversampled_values.EXT_AD_2 * REFERENCE_VOLTAGE_mV) >> 11;
    
    printf("Measured Values:\n");
    printf("U_BAT = %d mV\n", measurements.battery_voltage_mV);
    printf("I_BAT = %d mA\n", measurements.battery_current_mA);
    printf("T_BAT = %d dgC\n", measurements.battery_temperature_dgC);
    printf("EXT_AD_1 = %d mV\n", measurements.EXT_ADC_1_mV);
    printf("EXT_AD_2 = %d mV\n", measurements.EXT_ADC_2_mV);
    printf("\n");
}

void print_ADC_input_voltages (void)
{
    struct {
        uint16_t I_MEAS;
        uint16_t T_MEAS;
        uint16_t U_MEAS;
        uint16_t EXT_AD_1;
        uint16_t EXT_AD_2;
    } oversampled_values = {0};
    
    struct {
        uint16_t I_MEAS;
        uint16_t T_MEAS;
        uint16_t U_MEAS;
        uint16_t EXT_AD_1;
        uint16_t EXT_AD_2;
    } measured_voltage_mV;
    
    uint it_sample;
    
    // Oversample ADC values
    //
    // Accumulate samples
    // 16 samples per channel
    // -> ENOB = ENOB_ADC + bits_oversampling = 9 + ld(16)/2 = 9 + 2 = 11
    for (it_sample = 0; it_sample < 16; it_sample += 1) {
        oversampled_values.I_MEAS += ADC_sample_buffer.I_MEAS[it_sample];
        oversampled_values.T_MEAS += ADC_sample_buffer.T_MEAS[it_sample];
        oversampled_values.U_MEAS += ADC_sample_buffer.U_MEAS[it_sample];
        oversampled_values.EXT_AD_1 += ADC_sample_buffer.EXT_AD_1[it_sample];
        oversampled_values.EXT_AD_2 += ADC_sample_buffer.EXT_AD_2[it_sample];
    }
    
    // Shift for correct number of effective bits
    //
    // Accumulator has bits_ADC + bits_accumulation = 10 + ld(16) = 14
    // -> shift by 3 to get 11 valid bits
    oversampled_values.I_MEAS >>= 3;
    oversampled_values.T_MEAS >>= 3;
    oversampled_values.U_MEAS >>= 3;
    oversampled_values.EXT_AD_1 >>= 3;
    oversampled_values.EXT_AD_2 >>= 3;

    // Convert ADC code to voltage
    //
    // Ui = (ADC_code * Uref) / (2**ENOB - 1)
    // Neglect - 1 for easier computation by bitshift
    measured_voltage_mV.I_MEAS = (oversampled_values.I_MEAS * REFERENCE_VOLTAGE_mV) >> 11;
    measured_voltage_mV.T_MEAS = (oversampled_values.T_MEAS * REFERENCE_VOLTAGE_mV) >> 11;
    measured_voltage_mV.U_MEAS = (oversampled_values.U_MEAS * REFERENCE_VOLTAGE_mV) >> 11;
    measured_voltage_mV.EXT_AD_1 = (oversampled_values.EXT_AD_1 * REFERENCE_VOLTAGE_mV) >> 11;
    measured_voltage_mV.EXT_AD_2 = (oversampled_values.EXT_AD_2 * REFERENCE_VOLTAGE_mV) >> 11;
    
    printf("Measured Voltages:\n");
    printf("I_MEAS = %d mV\n", measured_voltage_mV.I_MEAS);
    printf("T_MEAS = %d mV\n", measured_voltage_mV.T_MEAS);
    printf("U_MEAS = %d mV\n", measured_voltage_mV.U_MEAS);
    printf("EXT_AD_1 = %d mV\n", measured_voltage_mV.EXT_AD_1);
    printf("EXT_AD_2 = %d mV\n", measured_voltage_mV.EXT_AD_2);
    printf("\n");
}