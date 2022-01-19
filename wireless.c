/*
 * wireless.c
 *
 *  Created on: Jun 16, 2021
 *      Author: dragy
 */

#include "wireless.h"

void setModeIdle(void) {
    uint8_t buf[1];

    buf[0] = RH_RF95_MODE_STDBY;
    // go into standby mode
    SPI_Master_WriteReg(RH_RF95_REG_01_OP_MODE, buf, 1);
}

void waitPacketSent(void) {
    uint8_t buf[1];
    bool transmitting = true;

    while(transmitting) {
        SPI_Master_ReadReg(RH_RF95_REG_01_OP_MODE, 1);
        CopyArray(SPI_ReceiveBuffer, buf, 1);
        // if the bit is not set, we're done transmitting
        if (buf[0] & RH_RF95_MODE_TX == 0) {
           transmitting = false;
        }
    }
}

/* All good to go*/
bool init_wireless(void) {

    uint8_t buf[1];

    buf[0] = RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE;

    SPI_Master_WriteReg(RH_RF95_REG_01_OP_MODE, buf, 1);

    __delay_cycles(160000); // delay 10 milliseconds for the module to go to sleep

    SPI_Master_ReadReg(RH_RF95_REG_01_OP_MODE, 1);
    CopyArray(SPI_ReceiveBuffer, buf, 1);

    if (buf[0] != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE))
        return false; // device not properly read/written to

    buf[0] = 0;
    // set up the 256 byte FIFO for read and write
    SPI_Master_WriteReg(RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, buf, 1);
    SPI_Master_WriteReg(RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, buf, 1);

    setModeIdle();

    // set up default configuration - no Sync Words in LORA Mode
    buf[0] = 0x72;
    SPI_Master_WriteReg(RH_RF95_REG_1D_MODEM_CONFIG1, buf, 1);
    buf[0] = 0x74;
    SPI_Master_WriteReg(RH_RF95_REG_1E_MODEM_CONFIG2, buf, 1);
    buf[0] = 0x04;
    SPI_Master_WriteReg(RH_RF95_REG_26_MODEM_CONFIG3, buf, 1);

    // set the preamble length
    buf[0] = 0x00;
    SPI_Master_WriteReg(RH_RF95_REG_20_PREAMBLE_MSB, buf, 1);
    buf[0] = 0x08; // default is 8
    SPI_Master_WriteReg(RH_RF95_REG_21_PREAMBLE_LSB, buf, 1);

    // set the frequency to 915 MHz
    const uint32_t frf = (915.0 * 1000000.0) / RH_RF95_FSTEP;
    buf[0] = (frf >> 16) & 0xff;
    SPI_Master_WriteReg(RH_RF95_REG_06_FRF_MSB, buf, 1);
    buf[0] = (frf >> 8) & 0xff;
    SPI_Master_WriteReg(RH_RF95_REG_07_FRF_MID, buf, 1);
    buf[0] = frf & 0xff;
    SPI_Master_WriteReg(RH_RF95_REG_08_FRF_LSB, buf, 1);

    // set the power consumption
    buf[0] = RH_RF95_PA_DAC_DISABLE;
    SPI_Master_WriteReg(RH_RF95_REG_4D_PA_DAC, buf, 1);
    buf[0] = RH_RF95_PA_SELECT | 11; // 11 stands for lowish power
    SPI_Master_WriteReg(RH_RF95_REG_09_PA_CONFIG, buf, 1);

    return true;
}

void wireless_send(uint8_t* data, uint8_t len) {

    // we don't need to wait on an outgoing message from this device - we're sending once every 10 minutes
    setModeIdle();

    uint8_t buf[1];

    buf[0] = 0;
    // Position at the beginning of the FIFO
    SPI_Master_WriteReg(RH_RF95_REG_0D_FIFO_ADDR_PTR, buf, 1);

    // The headers of the transaction - I don't know the proper protocol for this
    buf[0] = 0xff;
    SPI_Master_WriteReg(RH_RF95_REG_00_FIFO, buf, 1);
    buf[0] = 0xff; // txHeaderFrom - maybe ID in here?
    SPI_Master_WriteReg(RH_RF95_REG_00_FIFO, buf, 1);
    buf[0] = 0x00;
    SPI_Master_WriteReg(RH_RF95_REG_00_FIFO, buf, 1);
    buf[0] = 0x00;
    SPI_Master_WriteReg(RH_RF95_REG_00_FIFO, buf, 1);

    // The message data
    SPI_Master_WriteReg(RH_RF95_REG_00_FIFO, data, len);
    buf[0] = len + RH_RF95_HEADER_LEN;
    SPI_Master_WriteReg(RH_RF95_REG_22_PAYLOAD_LENGTH, buf, 1);

    buf[0] = RH_RF95_MODE_TX;
    SPI_Master_WriteReg(RH_RF95_REG_01_OP_MODE, buf, 1);

    bool transmitting = true;

    __delay_cycles(10000); // delay 10 ms so the transaction can take place

    // busy wait while we're transmitting - we need the main CPU to copy the array over so don't bother going into LPM0
    while(transmitting) {
        // read the interrupt register
        SPI_Master_ReadReg(RH_RF95_REG_12_IRQ_FLAGS, 1);
        CopyArray(SPI_ReceiveBuffer, buf, 1);

        // check if the transmission is done
        if (buf[0] & RH_RF95_TX_DONE) {
            transmitting = false;
        }

        buf[0] = 0xff;
        SPI_Master_WriteReg(RH_RF95_REG_12_IRQ_FLAGS, buf, 1); // clear the flags

        __delay_cycles(1000);
    }

    setModeIdle(); // go back into idle mode
}

uint16_t add_crc(uint8_t buffer[], uint16_t bufferLength, uint8_t crcType){
    uint8_t i;
      uint16_t crc;
      uint16_t polynomial;

      polynomial = (crcType == IBM) ? 0xC002 : 0x8810; //0x8005 : 0x1021;
      crc = (crcType == IBM) ? 0xFFFF : 0x1D0F;

      for(i = 0; i < bufferLength; i++){
        crc = ComputeCrc(crc, buffer[i], polynomial);
      }
      if(crcType == IBM){
        return crc;
      }
      else{
        return (uint16_t)(~crc);
      }
}

uint16_t ComputeCrc (uint16_t crc, uint8_t data, uint16_t poly){
  uint8_t i;
  for (i = 0; i < 8; i++){
    if( ( ( ( crc & 0x8000 ) >> 8 ) ^ (data & 0x80) ) != 0 ){
      crc <<= 1;
      crc ^= poly;
      }
      else{
        crc <<= 1;
      }
    data <<= 1;
  }
  return crc;
}
