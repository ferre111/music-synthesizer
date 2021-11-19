/*
 * encoder.c
 *
 *  Created on: Jul 11, 2021
 *      Author: karol
 */

#include "encoder.h"

#include "main.h"
#include "tim.h"

//--------------------------------------------------------------------------------

typedef struct encoder_T
{
    int16_t count;
    int16_t speed;
    int16_t position;
    void(*encoder_inc)(void);
    void(*encoder_dec)(void);
} encoder;

//--------------------------------------------------------------------------------

static encoder encoder_ctx = {.encoder_inc = utility_BlankFun, .encoder_dec = utility_BlankFun};

//--------------------------------------------------------------------------------

void encoder_process(void)
{
    static int16_t position = 0;
    if ((encoder_ctx.position - position) >= 1)
    {
        encoder_ctx.encoder_inc();
    }

    if ((encoder_ctx.position - position) <= -1)
    {
        encoder_ctx.encoder_dec();
    }

    position = encoder_ctx.position;
}

//--------------------------------------------------------------------------------

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4)
    {
        encoder_ctx.count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
        encoder_ctx.position = encoder_ctx.count / 4;
    }
}

//--------------------------------------------------------------------------------

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//    static int16_t count = 0;
//    if (htim == &htim3)
//    {
//        encoder_ctx.speed = encoder_ctx.count - count;
//        count = encoder_ctx.count;
//    }
}
