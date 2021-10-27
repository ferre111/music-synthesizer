/*
 * utility.h
 *
 *  Created on: Oct 14, 2021
 *      Author: karol
 */

#ifndef APP_UTILITY_H_
#define APP_UTILITY_H_

#define ERROR_INDICATE_TIME 2500 //how long after error detection err_led should be turn on

void utility_ErrLedOn(void);
void utility_ErrLedOff(void);
void utility_TimeMeasurmentsSetHigh(void);
void utility_TimeMeasurmentsSetLow(void);

#endif /* APP_UTILITY_H_ */
