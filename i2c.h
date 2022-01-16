/*
 * i2c.h
 *
 *  Created on: Jun 1, 2021
 *      Author: Adam Nygard
 */
#ifndef I2C_H_
#define I2C_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum I2C_ModeEnum{
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE_MSB,
    TX_REG_ADDRESS_MODE_LSB,
    RX_REG_ADDRESS_MODE,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITCH_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;

#define MAX_BUFFER_SIZE     20

extern uint8_t ReceiveBuffer[MAX_BUFFER_SIZE];

I2C_Mode I2C_Master_ReadIntoBuffer(uint8_t dev_addr, uint8_t count);

I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint16_t reg_addr, bool long_addr, uint8_t count);

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint16_t reg_addr, bool long_addr, uint8_t *reg_data, uint8_t count);

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);

void initGPIO_I2C();

void initClockTo1MHz();

void initI2C();

#endif /* I2C_H_ */
