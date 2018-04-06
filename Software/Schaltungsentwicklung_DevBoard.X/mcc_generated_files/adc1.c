
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

#include "../main.h"

#include <stdint.h>

#include <xc.h>
#include "adc1.h"

/**
  Section: Driver Interface
*/

ADC_Sample_Buffer_t ADC_sample_buffer = {0};

void ADC1_Initialize (void)
{
    // ASAM enabled; DONE disabled; CLRASAM disabled; FORM Integer 16 bit; SAMP disabled; SSRC Internal counter; SIDL disabled; ON disabled; 
   AD1CON1 = 0xE4;

    // CSCNA enabled; ALTS disabled; BUFM disabled; SMPI 5; OFFCAL disabled; VCFG VREF+/AVSS; 
   AD1CON2 = 0x2410;

    // SAMC 25; ADRC PBCLK; ADCS 1; 
   AD1CON3 = 0x1901;

    // CH0SA AN5; CH0SB AN0; CH0NB Vrefl; CH0NA Vrefl; 
   AD1CHS = 0x50000;

    // CSSL15 disabled; CSSL14 disabled; CSSL11 disabled; CSSL10 disabled; CSSL13 disabled; CSSL9 disabled; CSSL12 disabled; CSSL0 disabled; CSSL8 disabled; CSSL7 disabled; CSSL6 enabled; CSSL5 enabled; CSSL4 enabled; CSSL3 enabled; CSSL2 enabled; CSSL1 disabled; 
   AD1CSSL = 0x7C;

   // Enabling ADC1 interrupt.
   IEC0bits.AD1IE = 1;
   
    AD1CON1bits.ADON = 1;
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
    
    for(count=0; count<=5; count++)
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

void __ISR ( _ADC_VECTOR, IPL1AUTO ) ADC_1 (void)
{
    static uint it_sequence = 0;
    
    // Read ADC Buffer since the interrupt is persistent
    
    ADC_sample_buffer.I_MEAS[it_sequence] = ADC1BUF0;
    ADC_sample_buffer.T_MEAS[it_sequence] = ADC1BUF1;
    ADC_sample_buffer.U_MEAS[it_sequence] = ADC1BUF2;
    ADC_sample_buffer.EXT_AD_1[it_sequence] = ADC1BUF3;
    ADC_sample_buffer.EXT_AD_2[it_sequence] = ADC1BUF4;
    
    it_sequence += 1;
    if(it_sequence >= 16)
        it_sequence = 0;
        
    // clear ADC interrupt flag
    IFS0CLR= 1 << _IFS0_AD1IF_POSITION;
}

/**
  End of File
*/
