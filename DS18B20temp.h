/*
 * DS18B20temp.h
 *
 *  Created on: Dec 12, 2021
 *      Author: haileyjohnson
 */

#ifndef DS18B20TEMP_H_
#define DS18B20TEMP_H_

#include "i2c.h"

//------------------------------------------------------------------------------
// Single-wire commands Registers
//------------------------------------------------------------------------------
#define ROM_SEARCH  0xF0
#define ROM_READ    0x33
#define ROM_MATCH   0x55
#define ROM_SKIP    0xCC
#define ROM_ALARM   0xEC

#define FUNC_CMD_CONVERT          0x44
#define FUNC_CMD_WRITE            0x4E
#define FUNC_CMD_READ             0xBE
#define FUNC_CMD_COPY             0x48
#define FUNC_CMD_RECALL           0xB8
#define FUNC_CMD_READ_PWR_SUPPLY  0x84

//------------------------------------------------------------------------------
// Define the SMCLK frequency
//------------------------------------------------------------------------------
#define CLKFREQ     4u                         // Timer clock frequency (Hz)
//------------------------------------------------------------------------------
// Define Single-wire Protocol Related Timing Constants
//------------------------------------------------------------------------------
#define RESET_LOW    (500 * CLKFREQ)            // Set line low (500us)
#define RESET_DETECT (50 * CLKFREQ)             // Reset detect (80us)
#define RESET_DELAY  (200 * CLKFREQ)            // Reset delay (360us) TEST CHANGE 60

#define WRITE_ONE    (13 * CLKFREQ)              // Write byte 1 (15us)
#define DELAY_ONE    (55 * CLKFREQ)             // Write delay 1 (64us)
#define WRITE_ZERO   (65 * CLKFREQ)             // Write byte 0 (55us)
#define DELAY_ZERO   (15 * CLKFREQ)             // Write delay 0 (10us)

#define READ_LOW     (5 * CLKFREQ)              // Set line low (6us)
#define READ_DETECT  (10 * CLKFREQ)              // Slave response (9us)
#define READ_DELAY   (62 * CLKFREQ)             // Slave release (60us)

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------
void init_DS18B20();

void ROM_search(); //F0h
void ROM_read();   //33h
void ROM_match();  //55h
void ROM_skip();   //CCh
void ROM_alarm();  //ECh

void function_command_convert();           //44h
void function_command_write();             //4Eh
void function_command_read();              //BEh
void function_command_copy();              //48h
void function_command_recall();            //B8h
void function_command_read_power_supply(); //84h

void Single_write(uint8_t Data);
uint8_t Single_read(void);

int DS18B20_temp_reading(uint8_t LSB, uint8_t MSB);

#endif /* DS18B20TEMP_H_ */
