/*============================================================================*/
#include "serial.h"
/*============================================================================*/
#include <stdint.h>

/*============================================================================*/
void Serial_1_Config(serial_config_t* serialConfig) {
    
    /*
     * 1. Initialize the SPBRGH:SPBRG registers for the appropriate baud rate.
     *     Set or clear the BRGH and BRG16 bits, as required, to achieve the 
     *     desired baud rate.
    */
    
    
    /*
     * BRG16: 16-Bit Baud Rate Register Enable bit
            1 = 16-bit Baud Rate Generator ? SPBRGH and SPBRG
            0 = 8-bit Baud Rate Generator ? SPBRG only (Compatible mode), SPBRGH value ignored
     */
    /*
     * BRGH: High Baud Rate Select bit
            Asynchronous mode: 1 = High speed
                               0 = Low speed
    */
    
    
    /* ======================================================================================
     * |SYNC | BRG16 | BRGH | BRG/EUSART MODE | BAUD RATE FORMULA  | SPBRGH:SPBRG            |
     * | 0   |   0   |   0  |   8bit - Assyc  | FOSC/[64 * (n + 1)]| n = FOSC/(baud*64) - 1  |
     * | 0   |   0   |   1  |   8bit - Assync | FOSC/[16 * (n + 1)]| n = FOSC/(baud*16) - 1  |
     * | 0   |   1   |   0  |   16bit- Assync | FOSC/[16 * (n + 1)]| n = FOSC/(baud*16) - 1  |
     * | 0   |   1   |   1  |   16bit- Assync | FOSC/[4 * (n + 1)] | n = FOSC/(baud*4) - 1   |
     * ======================================================================================
    */
    
    
    uint8_t brg8LOW;
    uint8_t brg8HIGH;
    uint8_t brg16HIGH;
    
    
    uint8_t error_8LOW;
    uint8_t error_8HIGH;
    uint8_t error_16HIGH;
    
    uint8_t error_min;
    
    // SPBRGH:SPBRG
    brg8LOW   = (_XTAL_FREQ/(serialConfig->serial_desired_baud * 64));
    brg8HIGH  = (_XTAL_FREQ/(serialConfig->serial_desired_baud * 16));
    brg16HIGH = (_XTAL_FREQ/(serialConfig->serial_desired_baud *  4));
    
    // ERROR CALCULATION
    error_8LOW   = (_XTAL_FREQ/(brg8LOW  * 64)) - serialConfig->serial_desired_baud;
    error_8HIGH  = (_XTAL_FREQ/(brg8HIGH * 16)) - serialConfig->serial_desired_baud;
    error_16HIGH = (_XTAL_FREQ/(brg16HIGH * 4)) - serialConfig->serial_desired_baud;
    
    // Select small error
    error_min = error_8LOW;
    
    BAUDCONbits.BRG16 = 0x00;
    TXSTAbits.BRGH = 0x00;
    
    SPBRG = (brg8LOW - 1);
    
    if(error_8HIGH < error_min)
    {
        error_min = error_8HIGH;
        
        BAUDCONbits.BRG16 = 0x00;
        TXSTAbits.BRGH = 0x01;
    
        SPBRG = (brg8HIGH - 1);
    }
    if(error_16HIGH < error_min)
    {
        BAUDCONbits.BRG16 = 0x01;
        TXSTAbits.BRGH = 0x01;
    
        SPBRG = (brg16HIGH - 1);
    }
    
    /*
     * 2. Enable the asynchronous serial port by clearing bit, SYNC, and 
     *     setting bit, SPEN.
     */
    
    /*SYNC: EUSART Mode Select bit
     *  1 = Synchronous mode
     *  0 = Asynchronous mode
     */

    TXSTAbits.SYNC = serialConfig->serial_op_mode;
    
    /*SPEN: Serial Port Enable bit
     * 1 = Serial port enabled (configures RX/DT and TX/CK pins as serial port pins)
     * 0 = Serial port disabled (held in Reset)
     */
    RCSTAbits.SPEN = 0x01;
    
    
    /*
     *4. If 9-bit transmission is desired, set transmit bit, TX9. Can be used 
     *     as address/data bit.
     * *  4. If 9-bit reception is desired, set bit, RX9.
     */
    
    /*
     * TX9: 9-Bit Transmit Enable bit
     *   1 = Selects 9-bit transmission
     *   0 = Selects 8-bit transmission
     * 
     * RX9: 9-Bit Receive Enable bit
     *   1 = Selects 9-bit reception
     *   0 = Selects 8-bit reception
     
    */
    
    if(serialConfig->serial_data_length == SERIAL_DATA_LENGTH_9)
    {
        TXSTAbits.TX9 = 0x01; 
        RCSTAbits.RC9 = 0x01; 
    }else
    {
        TXSTAbits.TX9 = 0x00;
        RCSTAbits.RC9 = 0x00; 
    }
    
    /*
     * 5. Enable the transmission by setting bit, TXEN, which will also set bit, TXIF.
     * 5. Enable the reception by setting bit, CREN.
    */
    
    /* 
     * TXEN: Transmit Enable bit(1)
     *  1 = Transmit enabled
     *  0 = Transmit disabled
     * 
     * CREN: Continuous Receive Enable bit
     *      Asynchronous mode: 1 = Enables receiver 
     *                         0 = Disables receiver
     *      Synchronous mode:  1 = Enables continuous receive until enable bit,
     *                             CREN, is cleared (CREN overrides SREN)
     *                         0 = Disables continuous receive
     */
    TXSTAbits.TXEN = 0x01;
    RCSTAbits.CREN = 0x01;
}
/*============================================================================*/
void Serial_Transmit(uint8_t data)
{
    TXREG = data;
}
/*============================================================================*/
uint8_t Serial_Receive(void)
{
    while(!RCIF);
    return RCREG;
}
/*============================================================================*/