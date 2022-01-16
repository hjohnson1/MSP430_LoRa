/*
 * spi.c
 *
 *  Created on: Jun 4, 2021
 *      Author: Adam Nygard
 */

#include "spi.h"

SPI_Mode SPI_MasterMode = SPI_IDLE_MODE;

/* The Register Address/Command to use*/
uint8_t SPI_TransmitRegAddr = 0;
uint8_t SPI_ReceiveBuffer[20] = {0};
uint8_t SPI_RXByteCtr = 0;
uint8_t SPI_ReceiveIndex = 0;
uint8_t SPI_TransmitBuffer[20] = {0};
uint8_t SPI_TXByteCtr = 0;
uint8_t SPI_TransmitIndex = 0;

void SendUCA0Data(uint8_t val)
{
    while (!(UCA0IFG & UCTXIFG));              // USCI_A0 TX buffer ready?
    UCA0TXBUF = val;
}

SPI_Mode SPI_Master_WriteReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    SPI_MasterMode = SPI_TX_REG_ADDRESS_MODE;
    SPI_TransmitRegAddr = reg_addr | 0x80; // write mask

    //Copy register data to TransmitBuffer
    CopyArray(reg_data, SPI_TransmitBuffer, count);

    SPI_TXByteCtr = count;
    SPI_RXByteCtr = 0;
    SPI_ReceiveIndex = 0;
    SPI_TransmitIndex = 0;

    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
    SendUCA0Data(SPI_TransmitRegAddr);

    __bis_SR_register(CPUOFF + GIE);              // Enter LPM0 w/ interrupts

    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    return SPI_MasterMode;
}

SPI_Mode SPI_Master_ReadReg(uint8_t reg_addr, uint8_t count)
{
    SPI_MasterMode = SPI_TX_REG_ADDRESS_MODE;
    SPI_TransmitRegAddr = reg_addr;
    SPI_RXByteCtr = count;
    SPI_TXByteCtr = 0;
    SPI_ReceiveIndex = 0;
    SPI_TransmitIndex = 0;

    SLAVE_CS_OUT &= ~(SLAVE_CS_PIN);
    SendUCA0Data(SPI_TransmitRegAddr);

    __bis_SR_register(CPUOFF + GIE);              // Enter LPM0 w/ interrupts

    SLAVE_CS_OUT |= SLAVE_CS_PIN;
    return SPI_MasterMode;
}

//******************************************************************************
// Device Initialization *******************************************************
//******************************************************************************

void initSPI()
{
    //Clock Polarity: The inactive state is high
    //MSB First, 8-bit, Master, 3-pin mode, Synchronous
    UCA0CTLW0 = UCSWRST;                       // **Put state machine in reset**
    UCA0CTLW0 |= UCCKPL | UCMSB | UCSYNC
                | UCMST | UCSSEL__SMCLK;
    UCA0BRW = 0x20; // TURN THIS DOWN TO 0x20 SINCE WE CAN RUN THIS FASTER
    UCA0CTLW0 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                          // Enable USCI0 RX interrupt
}


void initGPIO_SPI()
{

    // Configure SPI
    P1SEL0 |= BIT5 | BIT6 | BIT7;

    SLAVE_CS_DIR |= SLAVE_CS_PIN;
    SLAVE_CS_OUT |= SLAVE_CS_PIN;
}

//******************************************************************************
// SPI Interrupt ***************************************************************
//******************************************************************************

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    uint8_t uca0_rx_val = 0;
    switch(__even_in_range(UCA0IV, USCI_SPI_UCTXIFG))
    {
        case USCI_NONE: break;
        case USCI_SPI_UCRXIFG:
            uca0_rx_val = UCA0RXBUF;
            UCA0IFG &= ~UCRXIFG;
            switch (SPI_MasterMode)
            {
                case SPI_TX_REG_ADDRESS_MODE:
                    if (SPI_RXByteCtr)
                    {
                        SPI_MasterMode = SPI_RX_DATA_MODE;   // Need to start receiving now
                        //Send Dummy To Start
                        __delay_cycles(2000000);
                        SendUCA0Data(DUMMY);
                    }
                    else
                    {
                        SPI_MasterMode = SPI_TX_DATA_MODE;        // Continue to transmision with the data in Transmit Buffer
                        //Send First
                        SendUCA0Data(SPI_TransmitBuffer[SPI_TransmitIndex++]);
                        SPI_TXByteCtr--;
                    }
                    break;

                case SPI_TX_DATA_MODE:
                    if (SPI_TXByteCtr)
                    {
                      SendUCA0Data(SPI_TransmitBuffer[SPI_TransmitIndex++]);
                      SPI_TXByteCtr--;
                    }
                    else
                    {
                      //Done with transmission
                      SPI_MasterMode = SPI_IDLE_MODE;
                      __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
                    }
                    break;

                case SPI_RX_DATA_MODE:
                    if (SPI_RXByteCtr)
                    {
                        SPI_ReceiveBuffer[SPI_ReceiveIndex++] = uca0_rx_val;
                        //Transmit a dummy
                        SPI_RXByteCtr--;
                    }
                    if (SPI_RXByteCtr == 0)
                    {
                        SPI_MasterMode = SPI_IDLE_MODE;
                        __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
                    }
                    else
                    {
                        SendUCA0Data(DUMMY);
                    }
                    break;

                default:
                    __no_operation();
                    break;
            }
            __delay_cycles(1000);
            break;
        case USCI_SPI_UCTXIFG:
            break;
        default: break;
    }
}
