
/**
  ADC1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    adc1.c

  @Summary
    This is the generated header file for the ADC1 driver using PIC32MX MCUs

  @Description
    This header file provides APIs for driver for ADC1.
    Generation Information :
        Product Revision  :  PIC32MX MCUs - pic32mx : v1.35
        Device            :  PIC32MX150F128D
        Driver Version    :  0.5
    The generated drivers are tested against the following:
        Compiler          :  XC32 1.42
        MPLAB 	          :  MPLAB X 3.55
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include <xc.h>
#include "adc1.h"

//! Buffer for DMA operation destination
//!
//! Order as defined in ADC1_CHANNEL
uint16_t sample_buffer[80] = {0};

uint16_t ADC1_measured_voltage_mV[5] = {0};

/**
  Section: Driver Interface
*/

void ADC1_Initialize (void)
{
    // ASAM enabled; DONE disabled; CLRASAM disabled; FORM Integer 16 bit; SAMP disabled; SSRC Clearing sample bit ends sampling and starts conversion; SIDL disabled; ON enabled;
   AD1CON1 = 0x8004;

    // CSCNA enabled; ALTS disabled; BUFM disabled; SMPI 1; OFFCAL disabled; VCFG VREF+/VREF-;
   AD1CON2 = 0x6400;

    // SAMC 0; ADRC PBCLK; ADCS 0;
   AD1CON3 = 0x0;

    // CH0SA AN0; CH0SB AN0; CH0NB Vrefl; CH0NA Vrefl;
   AD1CHS = 0x0;

    // CSSL15 disabled; CSSL14 disabled; CSSL11 disabled; CSSL10 disabled; CSSL13 disabled; CSSL9 disabled; CSSL12 disabled; CSSL0 disabled; CSSL8 disabled; CSSL7 disabled; CSSL6 enabled; CSSL5 enabled; CSSL4 enabled; CSSL3 enabled; CSSL2 enabled; CSSL1 disabled;
   AD1CSSL = 0x7C;


   // Enabling ADC1 interrupt.
   IEC0bits.AD1IE = 1;
}

void ADC1_Start(void)
{
   AD1CON1bits.SAMP = 1;
}

void ADC1_Stop(void)
{
   AD1CON1bits.SAMP = 0;
}

void ADC1_ConversionResultBufferGet(uint32_t *buffer)
{
    int count;
    uint32_t *ADC32Ptr;

    ADC32Ptr = (uint32_t *)&(ADC1BUF0);

    for(count=0; count<=1; count++)
    {
        buffer[count] = (uint32_t)*ADC32Ptr;
        ADC32Ptr = ADC32Ptr + 4;
    }
}

uint32_t ADC1_ConversionResultGet(void)
{
    return ADC1BUF0;
}

bool ADC1_IsConversionComplete( void )
{
    return AD1CON1bits.DONE; //Wait for conversion to complete
}

void ADC1_ChannelSelect( ADC1_CHANNEL channel )
{
    AD1CHS = channel << 16;
}

void ADC1_update_measurements (void)
{
    uint it_ch, it_sample;
    uint oversampled_value;

    // Iterate over channels
    for (it_ch = 0; it_ch < ADC1_MAX_CHANNEL_COUNT; it_ch += 1) {
        oversampled_value = 0;
        // Accumulate samples
        // 16 per channel -> accumulator has now ENOB = ENOB_ADC + bits_Oversampling = ENOB_ADC + ld(16) / 2 = 9 + 2 = 11 b
        for (it_sample = it_ch; it_sample < 80; it_sample += ADC1_MAX_CHANNEL_COUNT)
            oversampled_value += sample_buffer[it_sample];
        // Shift for correct number of effective bits
        // Accumulator has 10 b + ld(16) b = 14 used bits -> shift by 3 b to get 11 b
        oversampled_value >>= 3;
        // Convert ADC code to voltage
        // Ui = (ADC_code * Uref) / (2**ENOB - 1)
        // Neglect - 1 for easier computation by bitshift
        ADC1_measured_voltage_mV[it_ch] = (oversampled_value * reference_voltage_mV) >> 11;
    }
}

void __ISR ( _ADC_VECTOR, IPL1AUTO ) ADC_1 (void)
{
    // Read ADC Buffer since the interrupt is persistent

    // clear ADC interrupt flag
    IFS0CLR= 1 << _IFS0_AD1IF_POSITION;
}

/**
  End of File
*/
