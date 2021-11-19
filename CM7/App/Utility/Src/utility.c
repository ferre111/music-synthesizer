/*
 * utility.c
 *
 *  Created on: Oct 14, 2021
 *      Author: karol
 */

#include "utility.h"
#include "main.h"
#include "tim.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define ERR_LED_GPIO_Port   ERR_LED_CM7_GPIO_Port
#define ERR_LED_Pin         ERR_LED_CM7_Pin

/*------------------------------------------------------------------------------------------------------------------------------*/

void utility_ErrLedOn(void)
{
    HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, 1U);
    /* Turn off error indicator in tim16 handler after 2s */
    __HAL_TIM_ENABLE(&htim16);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void utility_ErrLedOff(void)
{
    HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, 0U);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void utility_TimeMeasurmentsSetHigh(void)
{
    HAL_GPIO_WritePin(TIME_MEASURMENT_GPIO_Port, TIME_MEASURMENT_Pin, 1U);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void utility_TimeMeasurmentsSetLow(void)
{
    HAL_GPIO_WritePin(TIME_MEASURMENT_GPIO_Port, TIME_MEASURMENT_Pin, 0U);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void utility_TimeMeasurmentsToggle(void)
{
    HAL_GPIO_TogglePin(TIME_MEASURMENT_GPIO_Port, TIME_MEASURMENT_Pin);
}
