/*============================================================================*/
// CONFIG1H
#pragma config OSC = HS 
#pragma config FCMEN = OFF
#pragma config IESO = OFF

// CONFIG2L
#pragma config PWRT = OFF
#pragma config BOREN = SBORDIS
#pragma config BORV = 3

// CONFIG2H
#pragma config WDT = OFF
#pragma config WDTPS = 32768

// CONFIG3H
#pragma config CCP2MX = PORTC
#pragma config PBADEN = OFF
#pragma config LPT1OSC = OFF
#pragma config MCLRE = ON

// CONFIG4L
#pragma config STVREN = ON
#pragma config LVP = OFF
#pragma config XINST = OFF

// CONFIG5L
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF

// CONFIG5H
#pragma config CPB = OFF
#pragma config CPD = OFF

// CONFIG6L
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF

// CONFIG6H
#pragma config WRTC = OFF
#pragma config WRTB = OFF
#pragma config WRTD = OFF

// CONFIG7L
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF

// CONFIG7H
#pragma config EBTRB = OFF
/*============================================================================*/
#include <xc.h>
/*============================================================================*/
#include "pic18f4520/timer/timer.h"
#include "pic18f4520/interrupt/interrupt.h"
/*============================================================================*/
#include "pic18f4520/serial/serial.h"
/*============================================================================*/
#include "pic18f4520/gpio/gpio.h"
#include "board/pinout/pinout.h"
/*============================================================================*/
timer_config_t timerConfig = {
    .timer_length = TIMER_LENGTH_16, 
    .timer_clk_src = TIMER_CLKO_SRC,
    .timer_transition = TIMER_TRANSITION_LOW_HIGH,
    .timer_prescaler_assign = TIMER_PRESCALER_IS_ASSIGNED,
    .timer_prescaler_value = TIMER_PRESCALER_256
};
/*============================================================================*/
void __interrupt() TC0INT(void){
     if (INTCONbits.TMR0IF == 0x01) {
        
      DIGITAL_PIN_TOGGLE(LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
      DIGITAL_PIN_TOGGLE(LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);
      
      TMR0 = 0xD9D9; // TMR0 = 0x00; 
      INTCONbits.T0IF = 0x00;   // Clean Timer Flag
    }
}

/*============================================================================*/
void main(void) {
    
    PIN_CONFIGURE_DIGITAL(PIN_OUTPUT, LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
    PIN_CONFIGURE_DIGITAL(PIN_OUTPUT, LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);
    
    PIN_DIGITAL_WRITE(PIN_LOW, LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
    PIN_DIGITAL_WRITE(PIN_HIGH,LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);
    
    
    Interrupt_GlobalEnable();
    Timer0_Config(&timerConfig);
    
    Serial_Config(9600);
    
    int i, j; 
    
    while(1){
        
        Serial_Transmit(0x41);
        for(i = 0; i < 200; i++){
            for(j = 0; j < 200; j++);
        }
        
        Serial_Transmit(0x42);
    }
    return;
}
/*============================================================================*/