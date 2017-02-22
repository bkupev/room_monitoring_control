/*
 * sensor_hal.h
 *
 *  Created on: 10.2.2017
 *      Author: blagoj.kupev
 */

#ifndef CUSTOM_DRIVERS_SENSOR_HAL_H_
#define CUSTOM_DRIVERS_SENSOR_HAL_H_

typedef enum param_e {
	PARAM_TEMPERATURE,
	PARAM_HUMIDITY
} param_t;

typedef struct _ext_sensor {
	uint8_t (*init_sensor)(void** context_p); // Init sensor and its context
	double (*get_value)(void ** context_p, param_t parameter);
	uint8_t (*set_property)(void ** context_p, uint8_t property, uint32_t value);
}ext_sensor_t;

#ifdef USE_SHT11
extern ext_sensor_t sht11_sensor;
#endif

#endif /* CUSTOM_DRIVERS_SENSOR_HAL_H_ */
