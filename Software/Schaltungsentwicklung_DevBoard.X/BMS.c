// TODO
// * Regelung für Ladestrom und -spannung
// * FSM für BMS bauen mit Logik, wann die Zelle getrennt wird

#include "BMS.h"

#include <stdio.h>

#include "mcc_generated_files/mcc.h"

#include "SOC.h"

BMS_Configuration_t BMS_configuration = {0};

struct {
    uint16_t battery_voltage_mV;
    int16_t battery_current_mA;
    int16_t battery_current_filtered_mA;
    int8_t battery_temperature_dgC;
    uint16_t EXT_ADC_1_mV;
    uint16_t EXT_ADC_2_mV;
    //! Accumulator for integration of transfered charge since startup.
    //! The accumulation is conducted on current level to avoid accumulation of integer division error.
    uint32_t current_accumulator_mA;
    uint16_t SOC_promille;
    uint16_t remaining_charge_time_min;
    uint16_t remaining_discharge_time_min;
} measurements = {0};

typedef enum {
    Init,
    Battery_Disconnected,
    Charging,
    Charged,
    Discharging,
    Discharged,
    Fault
} BMS_State_t;

BMS_State_t current_state;

void BMS_init (void)
{
    current_state = Init;

    U_SENS_DISABLE_SetLow();

    delay_ms(5);

    update_measurements();

    measurements.SOC_promille = get_soc_by_ocv(measurements.battery_voltage_mV);
    measurements.current_accumulator_mA = measurements.SOC_promille * (BMS_configuration.cell_capacity_mAs / 50);
}

void BMS_step (void)
{
    static BMS_State_t next_state = Init;

    static Tick_t fault_timer_ms;

    bool enable_charging = false;
    bool enable_led = false;

    update_measurements();

    // BMS FSM
    // Update current state
    // @todo uncomment this
    current_state = next_state;

    switch (current_state) {
        case Init: {
            if (measurements.battery_voltage_mV >= BMS_configuration.max_battery_voltage_mV + 50) {
                // Battery overcharged
                next_state = Fault;
            } else if (measurements.battery_voltage_mV <= BMS_configuration.min_battery_voltage_mV - 50) {
                // Battery drained
                next_state = Fault;
            } else if (measurements.battery_voltage_mV <= 100) {
                next_state = Battery_Disconnected;
            } else {
                // Battery voltage normal
                if (DETECT_5V_GetValue() ) {
                    next_state = Charging;
                    printf("To Charging\n");
                } else {
                    next_state = Discharging;
                    printf("To Discharging\n");
                }
            }
        } break;
        case Battery_Disconnected: {
            enable_charging = false;
            enable_led = true;
        }
        case Charging: {
            // @todo adjust threshold for charging to charged
            // add timer for this purpose
            if (measurements.battery_voltage_mV >= BMS_configuration.end_of_charge_voltage_mV + 50) {
                next_state = Charged;
            } else if (!DETECT_5V_GetValue() ) {
                next_state = Discharging;
            } else {
                next_state = Charging;
            }

            enable_led = true;
            enable_charging = true;
        } break;
        case Charged: {
            // @todo 5V
            if (measurements.battery_voltage_mV <= BMS_configuration.end_of_charge_voltage_mV - 100) {
                next_state = Charging;
            } else if (!DETECT_5V_GetValue() ) {
                next_state = Discharging;
            } else {
                next_state = Charged;
            }

            enable_charging = false;
            enable_led = true;
        } break;
        case Discharging: {
            // @todo 5 V to top
            if (measurements.battery_voltage_mV <= BMS_configuration.min_battery_voltage_mV) {
                next_state = Discharged;
            } else if (DETECT_5V_GetValue() ) {
                next_state = Charging;
            } else {
                next_state = Discharging;
            }

            enable_led = true;
        } break;
        case Discharged: {
            GPIO_8_SetHigh();
            delay_ms(5000);
            BAT_DISCONNECT_SetHigh();
        } break;
        case Fault: {
            Error_Handler();
        }
        default: Error_Handler();
    }

    control_led (enable_led);
    control_charger (enable_charging);

    // Prints
    {
        static Tick_t last_tick_ms = 0;

        if (tick_ms - last_tick_ms >= 1000) {
            printf("Current State = %s");
            printf("Measured Values:\n");
            printf("U_BAT = %d mV\n", measurements.battery_voltage_mV);
            printf("I_BAT = %d mA\n", measurements.battery_current_mA);
            printf("T_BAT = %d dgC\n", measurements.battery_temperature_dgC);
            printf("Accumulated_Current = %d mA\n", measurements.current_accumulator_mA);
            printf("SOC = %d permille\n", measurements.SOC_promille);
            printf("Remaining time = %d\n", current_state == Discharging ? measurements.remaining_discharge_time_min : measurements.remaining_charge_time_min);

            if (current_state == Discharging) {
                printf("Remaining time = %d\n", measurements.remaining_discharge_time_min);
            } else if (current_state == Charging) {
                printf("Remaining time = %d\n", measurements.remaining_charge_time_min);
            }

            printf("\n");

            last_tick_ms = tick_ms;
        }
    }

    // Funktion implementieren
    // voltage limit und current limit sind abhängig von BMS Konfiguration und derating wegen Temperatur
    // control_charging(enable_charging, voltage_limit, current_limit);


    // Check sensor values against defined limits
    // voltage_limits_violated = measurements.battery_voltage_mV < BMS_configuration.min_battery_voltage_mV || measurements.battery_voltage_mV > BMS_configuration.max_battery_voltage_mV;
    // current_limits_violated = measurements.battery_current_mA < BMS_configuration.min_battery_current_mA || measurements.battery_current_mA > BMS_configuration.max_battery_current_mA;
    // temperature_limits_violated = measurements.battery_temperature_dgC < BMS_configuration.min_battery_temperature_dgC || measurements.battery_temperature_dgC > BMS_configuration.max_battery_temperature_dgC;
}

static void update_measurements (void)
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
    // Gain = 56 m�C
    // Offset = 0�C -> 897
    measurements.battery_temperature_dgC = ( ( (int32_t)oversampled_values.T_MEAS - 897) * 56) / 1000;

    // External ADCs
    //
    // Gain = 1
    // Offset = 0
    measurements.EXT_ADC_1_mV = (oversampled_values.EXT_AD_1 * REFERENCE_VOLTAGE_mV) >> 11;
    measurements.EXT_ADC_2_mV = (oversampled_values.EXT_AD_2 * REFERENCE_VOLTAGE_mV) >> 11;

    // Accumulate current every 50 ms
    {
        static Tick_t last_tick_ms = 0;

        if (tick_ms - last_tick_ms >= 50) {
            // Limit underflow to zero
            if (-measurements.battery_current_mA > (int32_t)measurements.current_accumulator_mA) {
                measurements.current_accumulator_mA = 0;
            } else {
                measurements.current_accumulator_mA += measurements.battery_current_mA;
            }

            last_tick_ms = tick_ms;
        }
    }

    // Calculate SOC and remaining charge every 1 s
    {
        static Tick_t last_tick_ms = 0;
        static int16_t battery_current_delayed_mA = 0;

        if (tick_ms - last_tick_ms >= 1000) {
            // Calculate SOC based on accumulated current and cell capacity
            // @todo add formula
            measurements.SOC_promille = ( measurements.current_accumulator_mA ) / ( BMS_configuration.cell_capacity_mAs / 50 );

            // Filter Battery Current with PT1 for smoother estimate of remaining discharge time.
            //
            // T = 1 s
            //
            // y * (1 - 0.3679/z) = u * 0.6321/z -> y = (u * 0.6321)
            measurements.battery_current_filtered_mA = (battery_current_delayed_mA * 6321 + measurements.battery_current_filtered_mA * 3679) / 10000;
            battery_current_delayed_mA = measurements.battery_current_mA;

            // Estimate remaining charge / discharge time based on SOC and battery current
            // The calculation is different for charging and discharging:
            // * Linear extrapolation is used to estimate the remaining discharge time
            // * CC / CV aware extrapolation is used to estimate the remaining charge time (@todo Ist noch nicht drin, bis jetzt lineare extrapolation für beides)
            // @todo
            // So anpassen, dass automatisch positive und negative str�me unterschieden werden.
            // Nicht mit Vorzeichenumkehr, sondern Rechnung mit signed integer und dann anhand des Vorzeichens entscheiden.
            switch (current_state) {
                case Discharging: {
                    // Handle division by zero
                    if (measurements.battery_current_filtered_mA == 0) {
                        measurements.remaining_discharge_time_min = measurements.current_accumulator_mA / 60 / 20;
                    } else {
                        measurements.remaining_discharge_time_min = ( measurements.current_accumulator_mA / ( -measurements.battery_current_filtered_mA ) ) / 60 / 20;
                    }
                } break;
                case Charging: {
                    // Handle division by zero
                    if (measurements.battery_current_filtered_mA == 0) {
                        measurements.remaining_charge_time_min = measurements.current_accumulator_mA / 60 / 20;
                    } else {
                        measurements.remaining_charge_time_min = ( measurements.current_accumulator_mA / measurements.battery_current_filtered_mA ) / 60 / 20;
                    }
                } break;
                // @todo what should be done in other cases?
            }

            last_tick_ms = tick_ms;
        }
    }
}

static void print_ADC_input_voltages (void)
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

static void control_led (bool enable_led)
{
    static uint16_t pwm_value = 0;
    static bool wraparound = false;

    if (GPIO_9_GetValue() && !wraparound) {
        pwm_value += pwm_value / 16 + 10;

        if (pwm_value >= 40000) {
            pwm_value = 0;
            wraparound = true;
        }
    } else if (!GPIO_9_GetValue() ) {
        wraparound = false;
    }

    if (enable_led) {
        OC5_PWMPulseWidthSet(pwm_value);
    } else {
        OC5_PWMPulseWidthSet(0);
    }
}

static void control_charger (bool enable_charging)
{
    bool hyst_controller_enabled;

    if (BMS_configuration.end_of_charge_voltage_mV < 4150) {
        hyst_controller_enabled = true;
    } else {
        hyst_controller_enabled = false;
    }

    if (enable_charging) {
        if (hyst_controller_enabled) {
            static bool charging = false;

            if (charging) {
                if (measurements.battery_voltage_mV >= BMS_configuration.end_of_charge_voltage_mV) {
                    charging = false;
                }

                CHG_DISABLE_SetLow();
                GPIO_6_SetHigh();
            } else {
                if (measurements.battery_voltage_mV < BMS_configuration.end_of_charge_voltage_mV - 20) {
                    charging = true;
                }

                CHG_DISABLE_SetHigh();
                GPIO_6_SetLow();
            }
        } else {
            CHG_DISABLE_SetLow();
            GPIO_6_SetHigh();
        }
    } else {
        CHG_DISABLE_SetHigh();
        GPIO_6_SetLow();
    }
}
