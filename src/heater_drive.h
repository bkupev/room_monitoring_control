/*
 * heater_drive.h
 *
 *  Created on: 15.11.2016
 *      Author: blagoj.kupev
 */

#ifndef HEATER_DRIVE_H_
#define HEATER_DRIVE_H_


void init_heater_drive(void);
uint8_t poll_heater_drive(float set_temperature, float measured_temperature);


#endif /* HEATER_DRIVE_H_ */
