//
// Created by vgol on 12/07/2023.
//

#ifndef DRV2605_HAL_I2C_H
#define DRV2605_HAL_I2C_H

#include "hal_types.h"

#define HAPTICS_INIT_PIN(X)         HalGPIOInit(X)
#define HAPTICS_ENABLE_PIN(ID, X)   HalGPIOset(ID, X)

#ifdef __cplusplus
extern "C" {
#endif

void HalI2CInit(uint8 address);

int HalI2CWrite(uint8 * buffer, uint16 length);

int HalSensorReadReg(uint8 addr, uint8 * buffer, uint16 length);

void Haptics_WaitUs(uint16 microSecs);

void HalGPIOInit(pinID_t pin_id);

void HalGPIOset(pinID_t pin_id, uint8 value);

#ifdef __cplusplus
}
#endif

#endif //DRV2605_HAL_I2C_H
