# MSP430_LoRa
This program is takes a temperature reading using one-wire commmunication, 
send the reading to the LoRa board using SPI which transmits to the reciever, 
then goes into low power mode for 10 minutes.

Pin Outputs:
p1.4            -> Temp sensor data line
p1.5 (SCK)      -> LoRa SCK (ICSO pins) 
p1.6(MISO/SOMI) -> LoRa MISO/SOMI (ICSP pins)
p1.7(MOSI/SIMO) -> LoRa MOSI/SIMO (ICSP pins)
15.1            -> LoRa D10 (chip select)

Vcc = 3.3V

[]MISO    Vcc
SCK       MOSI
Reset     GND
