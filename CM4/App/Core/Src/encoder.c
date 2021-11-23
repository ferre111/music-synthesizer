/*
 * encoder.c
 *
 *  Created on: Jul 11, 2021
 *      Author: karol
 */

#include "encoder.h"
#include "utility.h"
#include "main.h"
#include "tim.h"

//--------------------------------------------------------------------------------

typedef struct encoder_T
{
    int16_t count;
    int16_t speed;
    int16_t position;
    callback_increment_fun increment_fun;
    callback_decrement_fun decrement_fun;
} encoder;

//--------------------------------------------------------------------------------

static encoder encoder_ctx = {.increment_fun = utility_BlankFun, .decrement_fun = utility_BlankFun};

//--------------------------------------------------------------------------------

void Encoder_process(void)
{
    static int16_t position = 0;
    if ((encoder_ctx.position - position) >= 1)
    {
        encoder_ctx.increment_fun();
    }

    if ((encoder_ctx.position - position) <= -1)
    {
        encoder_ctx.decrement_fun();
    }

    position = encoder_ctx.position;
}

//--------------------------------------------------------------------------------

void Encoder_set_callback_increment_fun(callback_increment_fun fun)
{
    encoder_ctx.increment_fun = fun;
}

//--------------------------------------------------------------------------------

void Encoder_set_callback_decrement_fun(callback_decrement_fun fun)
{
    encoder_ctx.decrement_fun = fun;
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
