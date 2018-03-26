#include "main.h"

#include <stdio.h>

#include "mcc_generated_files/mcc.h"
#include "BMS.h"

void main (void)
{
    uint32_t tick_last = 0;
    
    SYSTEM_Initialize();
    
    __XC_UART = 1;
    
    delay_ms(10);
    
    printf("Startup\n");
    
    U_SENS_DISABLE_SetLow();
    
    for(;;) {
        // Delay 1 s
        delay_ms(1000);
        
        printf("Measuring tick = %d\n", tick);
        print_ADC_input_voltages();
        update_measurements();
    }
}

/*void main (void)
{
    bool outputs_on = false;
    bool outputs_switched = false;
    
    bool run_down = false;
    
    uint32_t last_tick = 0;
    uint16_t pwm_value = 150;
    
    // initialize the device
    SYSTEM_Initialize();
    
    // Redirect printf to UART1
    __XC_UART = 1;
    
    OC1_PWMPulseWidthSet(TMR3_Period16BitGet());
    OC4_PWMPulseWidthSet(0);
    OC5_PWMPulseWidthSet(TMR2_Period16BitGet());
    
    U_SENS_DISABLE_SetLow();
    
    printf("-------\n");
    printf("Startup\n");
    printf("-------\n");

    while (1)
    {                   
        if (outputs_on) {
            GPIO_1_SetHigh();          
            
            if (tick % 10000 == 0) {
                if (!outputs_switched) {
                    outputs_on = false;
                    outputs_switched = true;
                    
                    //OC4_PWMPulseWidthSet(pwm_value);
                    printf("PWM Value = %d\n", pwm_value);
                    
                    if (run_down) {
                        if (pwm_value != 0) {
                            pwm_value -= 10;
                        }
                        else {
                            run_down = false;
                        }
                    }
                    else {
                        if (pwm_value < 2400) {
                            pwm_value += 10;
                        }
                        else {
                            run_down = true;
                        }
                    }
                }                
            }
            else {
                outputs_switched = false;
            }
        }
        else {
            GPIO_1_SetLow();                
            
            if (tick % 10000 == 0) {
                if (!outputs_switched) {
                    outputs_on = true;
                    outputs_switched = true;
                }                
            }
            else {
                outputs_switched = false;
            }
        }
    }
}*/