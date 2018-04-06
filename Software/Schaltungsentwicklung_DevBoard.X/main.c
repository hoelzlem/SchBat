#include "main.h"

#include <stdio.h>

#include "mcc_generated_files/mcc.h"
#include "BMS.h"

void main (void)
{
    uint32_t tick_last = 0;
    
    SYSTEM_Initialize();
    
    __XC_UART = 1;
    
    OC4_PWMPulseWidthSet(0);
    OC5_PWMPulseWidthSet(0);
    
    delay_ms(10);
    
    printf("Startup\n");
    
    printf("BMS_init\n");
    
    BMS_configuration.cell_capacity_mAs = 2050 * 3600;
    BMS_configuration.max_battery_voltage_mV = 4250;
    BMS_configuration.min_battery_voltage_mV = 3000;
    BMS_configuration.end_of_charge_voltage_mV = 4200;
    
    BMS_init();
    
    printf("Finished\n");
    
    for(;;) {
        
        // Heartbeat
        {
            static Tick_t last_tick_ms = 0;
            
            if (tick_ms - last_tick_ms >= 500) {
                GPIO_1_Toggle();
                
                last_tick_ms = tick_ms;
            }
        }
        
        // BMS_step
        {
            static Tick_t last_tick_ms = 0;
            
            if (tick_ms - last_tick_ms >= 20) {                
                BMS_step();

                last_tick_ms = tick_ms;
            }
        }
    }
}

void Error_Handler (void)
{    
    for (;;) {
        GPIO_8_Toggle();
        delay_ms(500);
    }
}