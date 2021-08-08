/*============================================================================*/
#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
/*============================================================================*/
#include "pic18f4520/fuse/fuse.h"
/*============================================================================*/
#include "pic18f4520/gpio/gpio.h"
#include "board/pinout/pinout.h"
/*============================================================================*/
#include "pic18f4520/timer/timer.h"
#include "pic18f4520/interrupt/interrupt.h"
#include "pic18f4520/serial/serial.h"
/*============================================================================*/
#include "board/board_definitions/board_definitions.h"
/*============================================================================*/
#include "app/main-app/main-app.h"
#include "app/bluetooth-hc-06/bluetooth_hc_06.h"
#include "app/display_lcd/display_lcd.h"
#include "app/dht11/dht11.h"
/*============================================================================*/
#include "pic18f4520/adc/adc.h"
#include "pic18f4520/eeprom/eeprom.h"
/*============================================================================*/
timer_config_t timerConfig = {
    .timer_length = TIMER_LENGTH_16,
    .timer_clk_src = TIMER_CLKO_SRC,
    .timer_transition = TIMER_TRANSITION_LOW_HIGH,
    .timer_prescaler_assign = TIMER_PRESCALER_IS_ASSIGNED,
    .timer_prescaler_value = TIMER_PRESCALER_2
};
/*============================================================================*/
serial_config_t serialConfig = {
    .serial_sync_com = SERIAL_ASSYNC_MODE,
    .serial_data_length = SERIAL_DATA_LENGTH_8,
    .serial_op_mode = SERIAL_MASTER_MODE,
    .serial_desired_baud = SPEED_SERIAL
};
/*============================================================================*/
adc_config_t adcConfig = {
    .adc_channel = CHANNEL_AN0,
    .negative_reference = INTERNAL_NEGATIVE_REFERENCE,
    .positive_reference = INTERNAL_POSITIVE_REFERENCE,
    .result_format = RIGHT_JUSTIFIED,
    .adc_clock = FOSC_8,
    .acquisition_time = TAD_2
};
/*============================================================================*/
extern global_timer_t global_timer_value;
/*============================================================================*/
uint8_t count = 0x00;
volatile char vector[3] = {0, 0, 0};
static uint8_t i = 0x00;
char byteReceived;
uint8_t firstReceive;
/*============================================================================*/
static global_timer_t t = 0;
extern bool TimeIsElapsed;

/*============================================================================*/
void tickHook_func(global_timer_t *timer_value) {
    (*timer_value)++;
    if ((*timer_value - t) >= TIME_TO_SEND_MS) {
        t = (*timer_value);
        TimeIsElapsed = true;
    }
}

/*============================================================================*/
void __interrupt() TC0INT(void) {


    // Interrup��o Timer 0
    if (INTCONbits.TMR0IF == 0x01) {

        tickHook_Execute(&global_timer_value);

        TMR0 = 0xFB1E; // TMR0 = 0x9E58; 
        INTCONbits.T0IF = 0x00; // Clean Timer Flag 
    }

    // Interrup��o recep��o serial
    if (PIR1bits.RCIF) {
        byteReceived = RCREG;

        // Os primeiros 4 bytes s�o descartados | HC envia Ready na conex�o
        if (firstReceive <= 0x04) {
            firstReceive++;
        } else {
            if (byteReceived != 0x3E) {
                vector[i] = byteReceived;
                i++;
            } else {
                vector[i] = byteReceived;
                Display_SendByte(DISPLAY_CLEAR, DISPLAY_COMMAND);
                __delay_ms(5);
                Display_WriteString(vector, sizeof (vector) + 1, 0);

                if (vector[1] == 0x43) {
                    User_SetState(true);
                    DIGITAL_PIN_TOGGLE(LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
                } else if (vector[1] == 0x44) {
                    User_SetState(false);
                    DIGITAL_PIN_TOGGLE(LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);
                }

                i = 0;
            }

            /*
             if(byteReceived == 0x45)
             {
                 DIGITAL_PIN_TOGGLE(LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
             }
             else if(byteReceived == 0x4b)
             {
                 DIGITAL_PIN_TOGGLE(LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);
             }
             */
        }


        PIR1bits.RCIF = 0x00;
    }
}
/*============================================================================*/
void main(void) {


    /*
    Interrupt_GlobalEnable();
    Timer0_Config(&timerConfig);
    Timer0_SetTickHook(tickHook_func);
*/
    Serial_1_Config(&serialConfig);

    // StartSystem(NULL);
    __delay_ms(300);
    DisplayLCD_Init();


    EEPROM_Erase();

    PIN_CONFIGURE_DIGITAL(PIN_OUTPUT, LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
    PIN_CONFIGURE_DIGITAL(PIN_OUTPUT, LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);

    PIN_DIGITAL_WRITE(PIN_LOW, LED_HEARTBEAT1_PORT, LED_HEARTBEAT1_MASK);
    PIN_DIGITAL_WRITE(PIN_LOW, LED_HEARTBEAT2_PORT, LED_HEARTBEAT2_MASK);

    PIN_CONFIGURE_DIGITAL(PIN_INPUT, VOLTAGE_INPUT_PORT, VOLTAGE_INPUT_MASK);
    
    
    uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
    uint16_t sum, RH, TEMP;
    uint8_t check = 0;
    
    uint8_t s = 0x00;
    __delay_ms(10000);
    
    while (1)
    {
        // main_application(NULL);
        DHT11_Start();
        DHT11_Check_Response();

        // Read 40 bits (5 Bytes) of data
        Rh_byte1 = read_data();
        Rh_byte2 = read_data();
        
        Temp_byte1 = read_data();
        Temp_byte2 = read_data();
        
        sum = read_data();
        
        if(sum != (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2))
        {
            Display_SendByte(DISPLAY_CLEAR, DISPLAY_COMMAND);
            __delay_ms(3);
            Display_WriteString("ERRO", 5, 0);
        }
        
        Display_SendByte(DISPLAY_CLEAR, DISPLAY_COMMAND);
        __delay_ms(3);
        Display_WriteString("HUMD:- ", 8, 0);
        Display_WriteByte((Rh_byte1 / 10) + 48); // print 1nd digit
        Display_WriteByte(((Rh_byte1 % 10)) + 48); // print 2st digit
        Display_WriteByte(0x2e); // print 2st digit
        Display_WriteByte((Rh_byte2 / 10) + 48); // print 1nd digit
        Display_WriteByte(((Rh_byte2 % 10)) + 48); // print 2st digit
        
        Display_WriteByte(0x20); // print 2st digit
        Display_WriteByte((sum / 10) + 48); // print 2st digit
        Display_WriteByte((sum % 10) + 48); // print 2st digit
        
        
        Display_SendByte((DISPLAY_DDRAM_ADD | DISPLAY_DDRAM_ADD_2_1), DISPLAY_COMMAND);
        Display_WriteString("TEMP:- ", 8, 0);
        Display_WriteByte((Temp_byte1 / 10) + 48); // print 1nd digit
        Display_WriteByte(((Temp_byte1 % 10)) + 48); // print 2st digit
        Display_WriteByte(0x2e); // print 2st digit
        Display_WriteByte((Temp_byte2 / 10) + 48); // print 1nd digit
        Display_WriteByte(((Temp_byte2 % 10)) + 48); // print 2st digit
        
        Display_WriteByte(0x2e); // print 2st digit
        Display_WriteByte(s + 48); // print 2st digit
        __delay_ms(4000);
        
        
        if( s < 9)
        {
            s++;
        }else 
        {
            s = 0;
        }
    }
    return;
}
/*============================================================================*/