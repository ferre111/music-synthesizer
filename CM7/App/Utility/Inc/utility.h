/*
 * utility.h
 *
 *  Created on: Oct 14, 2021
 *      Author: karol
 */

#ifndef APP_UTILITY_H_
#define APP_UTILITY_H_

#define Q31_MULTIPLICATION(X, Y) (q31_t)(((q63_t)(X) * (q63_t)(Y)) / (1U << 31U))
#define Q31_DIVISION(X, Y) (q31_t)((q63_t)(X) * (1U << 31U) / (Y))

void utility_ErrLedOn(void);
void utility_ErrLedOff(void);
void utility_LoadLedOn(void);
void utility_LoadLedOff(void);
void utility_TimeMeasurmentsSetHigh(void);
void utility_TimeMeasurmentsSetLow(void);
void utility_TimeMeasurmentsToggle(void);

#endif /* APP_UTILITY_H_ */
