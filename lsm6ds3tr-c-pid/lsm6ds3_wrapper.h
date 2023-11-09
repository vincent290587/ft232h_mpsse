//
// Created by vgol on 02/11/2023.
//

#ifndef NRF_TEMPLATE_LSM6DS3_WRAPPER_H
#define NRF_TEMPLATE_LSM6DS3_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void lsm6ds3tr_c_init(void);

void lsm6ds3tr_c_read_data_polling(void);

#ifdef __cplusplus
}
#endif

#endif //NRF_TEMPLATE_LSM6DS3_WRAPPER_H
