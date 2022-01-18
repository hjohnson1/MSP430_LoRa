/*
 * timer.c
 *
 *  Created on: Jun 9, 2021
 *      Author: Adam Nygard
 */
#include "timer.h"

void init_timer(void) {
    P2SEL1 |= BIT6 | BIT7;                  // P2.6~P2.7: crystal pins
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);      // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);

    // RTC count re-load compare value at 32.
    // 1024/32768 * 19200 = 600 sec -> 10 minutes.
    RTCMOD = 100;//19200;//1000 for testing gives 31 seconds.

    // Source = 32kHz crystal, divided by 1024
    RTCCTL = RTCSS__XT1CLK | RTCSR | RTCPS__1024 | RTCIE;
}

void sleep_pins(void) {

    P1DIR = 0xff;//|= 0xf3; // don't touch SCL or SDA (NOTE: Not using for this project)
    P2DIR = 0xff;
    P3DIR = 0xff;
    P4DIR = 0xff;
    P5DIR = 0xff;
    P6DIR = 0xff;

    P1OUT = 0x00;//10; //00010000 P1.4 temp sensor
    P2OUT = 0x00;
    P3OUT = 0x00;//50; //01010000
    P4OUT = 0x00;//20; //00100000
    P5OUT = 0x00;//04; //00000100
    P6OUT = 0x00;

    UCB0CTLW0 = UCSWRST; // put euscib0 into reset
    UCA0CTLW0 = UCSWRST; // put euscia0 into reset
}

void sleep_10_min(void) {

    sleep_pins();

    init_timer();

    __bis_SR_register(LPM4_bits | GIE);     // Enter LPM3, enable interrupt
}

void init_timer_B_open(void) {
    TB0CTL = TBCLR; // reset the peripheral
    TB0CTL |= TBSSEL__ACLK | MC__UP;
    TB0CCR0 = 713; // 21.75ms --> (1/32768) * 713
    TB0CCR1 = 57; // 1.75ms --> (1/32768) * 57
    TB0CCTL0 |= CCIE;
    TB0CCTL1 |= CCIE;
    __enable_interrupt();
    TB0CCTL0 &= ~CCIFG;
    TB0CCTL1 &= ~CCIFG;
}

void init_timer_B_close(void) {
    TB0CTL = TBCLR; // reset the peripheral
    TB0CTL |= TBSSEL__ACLK | MC__UP;
    TB0CCR0 = 680; // 20.75ms --> (1/32768) * 680
    TB0CCR1 = 25; // .75ms --> (1/32768) * 25
    TB0CCTL0 |= CCIE;
    TB0CCTL1 |= CCIE;
    __enable_interrupt();
    TB0CCTL0 &= ~CCIFG;
    TB0CCTL1 &= ~CCIFG;
}
