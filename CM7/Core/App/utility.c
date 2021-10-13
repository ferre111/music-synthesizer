/*
 * utility.c
 *
 *  Created on: Oct 14, 2021
 *      Author: karol
 */

#include "utility.h"
#include "main.h"

static uint32_t err_timestamp;

void utility_ErrLedOn(void)
{
    err_timestamp = HAL_GetTick();
    HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, 1U);
}

void utility_ErrLedOff(void)
{
    if ((HAL_GetTick() - err_timestamp) > ERROR_INDICATE_TIME)
    {
        HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, 0U);
    }
}


