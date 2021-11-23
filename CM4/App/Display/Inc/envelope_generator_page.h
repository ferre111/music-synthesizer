/*
 * envelope_generator_page.h
 *
 *  Created on: Nov 18, 2021
 *      Author: karol
 */

#ifndef DISPLAY_INC_ENVELOPE_GENERATOR_PAGE_H_
#define DISPLAY_INC_ENVELOPE_GENERATOR_PAGE_H_

#include <stdio.h>
#include "OLED.h"

void envelope_generator_page_init(void);
void envelope_generator_page_draw(void);
void envelope_generator_encoder_enter_press_fun(void);
void envelope_generator_page_exit(void);

#endif /* DISPLAY_INC_ENVELOPE_GENERATOR_PAGE_H_ */
