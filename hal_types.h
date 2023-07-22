//
// Created by vgol on 12/07/2023.
//

#ifndef DRV2605_HAL_TYPES_H
#define DRV2605_HAL_TYPES_H


#include <stdint.h>
#include <stdbool.h>

#define uint8   uint8_t
#define uint16  uint16_t
#define uint32  uint32_t

#define u8   uint8_t
#define u16  uint16_t

#define pinID_t uint16

#define LOG(...)    printf(__VA_ARGS__)

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif //DRV2605_HAL_TYPES_H
