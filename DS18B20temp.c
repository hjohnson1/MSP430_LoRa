/*
 * DS18B20temp.c
 *
 *  Created on: Dec 12, 2021
 *      Author: haileyjohnson
 */

#include "DS18B20temp.h"

/***************************************************
 * function: init_ds18B20()
 *
 * input: void
 *
 * Description: Perform single wire reset
 * - send reset pulse by pulling low for RESET_LOW
 * - Pull high
 * - Wait
 * - Follow by ROM cmd then a function cmd
****************************************************/
void init_DS18B20(){
    __bis_SR_register(SCG0);                      // disable FLL
      CSCTL3 |= SELREF__REFOCLK;                    // Set REFO as FLL reference source
      CSCTL0 = 0;                                   // clear DCO and MOD registers
      CSCTL1 &= ~(DCORSEL_7);                       // Clear DCO frequency select bits first
      CSCTL1 |= DCORSEL_2; //4                         // Set DCO = 16MHz
      CSCTL2 = FLLD_0 + 121;                        // DCODIV = 16MHz
      __delay_cycles(3);
      __bic_SR_register(SCG0);                      // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));    // Poll until FLL is locked

    P1OUT = 0x00;                               // P1.5 driven low
    __delay_cycles(RESET_LOW);                  // Set delay, __delay_cycles are used to save memory space. Consider using TimerB at 4 MHz with interrupts.
    P1DIR = 0x00;                               // P1.5 input
    __delay_cycles(RESET_DELAY);                // Set delay
    P1OUT = BIT4;                               // Reset P1.5 to output high
    P1DIR = BIT4;
}

void ROM_search(){
    //one slave, use read
}

/***************************************************
 * function: ROM_read()
 *
 * input: void
 *
 * Description: Request Identification number from
 * slave
 *
 * NOTE: Only used for one slave connected, otherwise
 * use ROM_search()
****************************************************/
void ROM_read(){
    Single_write(ROM_READ);
}

void ROM_match(){
    //cmd followed by code
    //can ask specific device
}

void ROM_skip(){
    //broadcast to all slaves
    //followed by READ scratchpad will fail if many slaves
}

void ROM_alarm(){
    //only set alarm flag will respond
    //follow by data exchange by returning to init
}

/**********************************************************
 * function: function_command_convert()
 *
 * input: void
 *
 * Description: sends Convert T command to sensor
 * - Must init first and send ROM cmd second
 * - 0 = in progress   1 = conversion done
 *********************************************************/
void function_command_convert(){
    Single_write(FUNC_CMD_CONVERT);
    while(!P1IN & BIT4);
}

void function_command_write(){
    //write 3 bytes
    //byte 1 -> Th reg
    //byte 2 -> Tl reg
    //byte 3 -> config reg
    //LSB first, all 3 before reset

    //write slot = 60 microsec + 1 microsec recovery

    //1 => pull bus low, within 15 microsec release
    //0 => pull bus low and keep low for at least 60 microsec
}

/**********************************************************
 * function: function_command_read()
 *
 * input: void
 *
 * Description: Requests to read from scratchpad.
 * - byte[0] = LSB of temp reading
 *     -(bit 1-4 decimal point, bit 5-8 LSB of temp)
 * - byte[1] = MSB of temp reading
 *********************************************************/
void function_command_read(){
    //starts with LSB 0
    //continues through 9 (byte 8 - CRC)
    //can issue reset at any time

    Single_write(FUNC_CMD_READ);
}

void function_command_copy(){
    //copies Th, Tl, and config to EEPROM

}

void function_command_recall(){
    //recalls alarm trigger values Th and Tl and config from EEPROM
    //places data in bytes 2,3,4
    //can issue read 0=in progress 1 = done

    //automatic at power up
}

void function_command_read_power_supply(){
    //master issues followed by read to determine if any sensors are on parasite power.
    //during read parasite pull low and external pulls high.
}

//------------------------------------------------------------------------------
// void Single_write(unsigned uint8_t Data)
//
// Write the 8-bit word 'Data' to the Single-wire enabled device.
//------------------------------------------------------------------------------
void Single_write(uint8_t Data)
{
    uint8_t bit;                                // Initialize bit counter
    for (bit = 0; bit < 8; bit++)               // Loop 8 bits to write a byte
    {
        P1OUT = 0x00;                           // Set P1.5 low
        if((Data >> bit) & 0x01)                // If bit is a 1
        {
            __delay_cycles(WRITE_ONE);          // Set delay
            P1OUT = BIT4;                       // Set P1.5 back to high
            __delay_cycles(DELAY_ONE);          // Set delay
        }
        else                                    // Else if a 0
        {
            __delay_cycles(WRITE_ZERO);         // Set delay
            P1OUT = BIT4;                       // Set P1.5 back to high
            __delay_cycles(DELAY_ZERO);         // Set delay
        }
    }
}

//------------------------------------------------------------------------------
// unsigned uint8_t Single_read(unsigned uint8_t Data)
//
// Read the 8-bit word 'Data' from the Single-wire enabled device
//------------------------------------------------------------------------------
uint8_t Single_read(void)
{
    uint8_t bit;                                // Initialize bit counter
    uint8_t Data = 0;                           // Initialize return data byte
    for (bit = 0; bit < 8; bit++)               // Loop 8 bits to read a byte
    {
        P1OUT = 0x00;                           // P1.5 driven low
        __delay_cycles(READ_LOW);               // Set delay
        P1DIR = 0x00;                           // P1.5 input
        __delay_cycles(READ_DETECT);            // Set delay
        if(P1IN & BIT4) Data |= (0x01 << bit);  // If high then read 1, else 0
        __delay_cycles(READ_DELAY);             // Set delay
        P1OUT = BIT4;                           // Reset P1.5 to output high
        P1DIR = BIT4;
    }
    return Data;                                // Return byte
}

int DS18B20_temp_reading(uint8_t LSB, uint8_t MSB){
    //MSB+LSB[8-5] . LSB[4-1]



}
