
# FT232X

## I2C wiring

- Connect FT232H D1 and D2 together with a jumper wire.  This combined connection is the I2C SDA data line.
- Add a 4.7 kilo-ohm resistor from the I2C SDA data line (pins D1 and D2 above) up to FT232H 5V.
- Add a 4.7 kilo-ohm resistor from FT232H D0 up to FT232H 5V.  This pin D0 is the I2C SCL clock line.

| Signal | Pin   |
|--------|-------|
| SCL    | D0    |
| SDA    | D1-D2 |


## SPI wiring

- Device SCLK or clock to FT232H D0 / serial clock.
- Device MOSI or data in to FT232H D1 / serial output.
- Device MISO or data out to FT232H D2 / serial input.

# FT2232H

Can do I2C and SPI at the same time (+ GPIO)


