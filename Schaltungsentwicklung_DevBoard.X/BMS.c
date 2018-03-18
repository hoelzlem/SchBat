#include "BMS.h"

#include <mcc_generated_files/adc1.h>

Measurements_t measurements = {0};

void BMS_calculate_measurements (void)
{
    // Ubat = Umeas * Gain = Umeas * 1.8
    measurements.battery_voltage_mV = ( (uint32_t)(ADC1_measured_voltage_mV[2]) * 18) / 10;
    // Ibat = Umeas * Gain + Offset = Umeas * 0.8772 mA/mV - 600 mA
    measurements.battery_current_mA = ((int32_t)(ADC1_measured_voltage_mV[0]) * 8772) / 10000 - 600;
    // Tbat = Umeas * Gain + Offset = Umeas * 0.04564 K/mV - 50 °C
    measurements.battery_temperature_dgC = ((int32_t)(ADC1_measured_voltage_mV[1]) * 4564) / 100000 - 50;
}
