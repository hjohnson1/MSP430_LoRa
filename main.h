/*
 * main.h
 *
 *  Created on: Jun 28, 2021
 *      Author: dragy
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
//
#include "spi.h"
//#include "i2c.h"

// sensors
//#include "seesaw.h"
//#include "pressure.h"
//#include "co2.h"
#include "DS18B20temp.h"

#include "wireless.h"

#include "timer.h"

extern bool i2c_succeed;
extern uint8_t wireless_buf[33];
extern uint8_t seesaw_buf[24];

#endif /* MAIN_H_ */
