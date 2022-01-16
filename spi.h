/*
 * spi.h
 *
 *  Created on: Jun 4, 2021
 *      Author: Adam Nygard
 */

#ifndef SPI_H_
#define SPI_H_

#include <msp430.h>
#include <stdint.h>

#define SLAVE_CS_OUT    P5OUT
#define SLAVE_CS_DIR    P5DIR
#define SLAVE_CS_PIN    BIT1

#define DUMMY   0xFF

typedef enum SPI_ModeEnum{
    SPI_IDLE_MODE,
    SPI_TX_REG_ADDRESS_MODE,
    SPI_RX_REG_ADDRESS_MODE,
    SPI_TX_DATA_MODE,
    SPI_RX_DATA_MODE,
    SPI_TIMEOUT_MODE
} SPI_Mode;

extern uint8_t SPI_ReceiveBuffer[20];

SPI_Mode SPI_Master_WriteReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count);

SPI_Mode SPI_Master_ReadReg(uint8_t reg_addr, uint8_t count);

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);

void SendUCB0Data(uint8_t val);

void initSPI();

void initGPIO_SPI();

#endif /* SPI_H_ */
