/*
 * button.h
 *
 *  Created on: Jan 17, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include <stdbool.h>
#include "main.h"

//--------------------------------------------------------------------------------

#define BUTTON_DEBOUNCE_TIME 20
#define BUTTON_HOLD_TIME     1000

//--------------------------------------------------------------------------------

/** @brief      Pointers to function for button. */
typedef void (*callback_press_button)(void);
typedef void (*callback_hold_button)(void);

//--------------------------------------------------------------------------------

typedef enum button_types_T
{
    BUTTON_TYPES_ENCODER,

    Button_types_end
} button_types;

//--------------------------------------------------------------------------------

/** @brief      Button structure describing all parameters. */
typedef struct button_T
{
    GPIO_TypeDef                *port;
    uint16_t                    pin;
    volatile uint32_t           start_press_time;
    volatile uint32_t           start_unpress_time;
    volatile bool               actual_pin_state;
    volatile bool               press;
    callback_press_button       press_button_fun;
    callback_hold_button        hold_button_fun;
} button;

//--------------------------------------------------------------------------------

/*
 * @brief   This function should be insert in main loop.
 */
void Buttons_process(void);

//--------------------------------------------------------------------------------

/** @brief   Set callback function for pressed button.
 *  @param   type - button type
 *  @param   fun - function
 */
void Buttons_set_callback_press_function(button_types type, callback_press_button fun);

//--------------------------------------------------------------------------------

/** @brief   Set callback function for holded button.
 *  @param   type - button type
 *  @param   fun - function
 */
void Buttons_set_callback_hold_function(button_types type, callback_hold_button fun);
void utility_BlankFun(void);
