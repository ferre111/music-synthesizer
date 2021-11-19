/*
 * button.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#include "buttons.h"
#include "utility.h"

//--------------------------------------------------------------------------------

static void Buttons_do_button_fun(void);
//static void blank_fun(void);

//--------------------------------------------------------------------------------

static button ctx[Button_types_end] =
{
            [BUTTON_TYPES_ENCODER] = {.port = ENCODER_BUTTON_GPIO_Port, .pin = ENCODER_BUTTON_Pin, .press_button_fun = utility_BlankFun, .hold_button_fun = utility_BlankFun},
};

//--------------------------------------------------------------------------------

void Buttons_set_callback_press_function(button_types type, callback_press_button fun)
{
    ctx[type].press_button_fun = fun;
}

//--------------------------------------------------------------------------------

void Buttons_set_callback_hold_function(button_types type, callback_hold_button fun)
{
    ctx[type].hold_button_fun = fun;
}

//--------------------------------------------------------------------------------

void Buttons_process(void)
{
    Buttons_do_button_fun();
}

//--------------------------------------------------------------------------------

void Buttons_EXTI_handler(void)
{
    for(uint8_t i = 0U; i < Button_types_end; i++)
    {
        ctx[i].actual_pin_state =  HAL_GPIO_ReadPin(ctx[i].port, ctx[i].pin);

        if (ctx[i].actual_pin_state == false)
        {
            if ((HAL_GetTick() - ctx[i].start_unpress_time) > BUTTON_DEBOUNCE_TIME)
            {
                ctx[i].start_press_time = HAL_GetTick();
                ctx[i].press = true;
            }
        }
        else
        {
            ctx[i].start_unpress_time = HAL_GetTick();
        }
    }
}

//--------------------------------------------------------------------------------

static void Buttons_do_button_fun(void)
{
    for(uint8_t i = 0U; i < Button_types_end; i++)
    {
        if (ctx[i].press == true)
        {
            if (ctx[i].actual_pin_state == true)
            {
                ctx[i].press = false;
                ctx[i].press_button_fun();
            }
            else if ((HAL_GetTick() - ctx[i].start_press_time) > BUTTON_HOLD_TIME)
            {
                ctx[i].press = false;
                ctx[i].hold_button_fun();
            }
        }
    }
}

//--------------------------------------------------------------------------------

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    Buttons_EXTI_handler();
}

