/*
 * encoder.h
 *
 *  Created on: Jul 11, 2021
 *      Author: karol
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

//--------------------------------------------------------------------------------

typedef void (*callback_increment_fun)(void);
typedef void (*callback_decrement_fun)(void);

//--------------------------------------------------------------------------------

void Encoder_process(void);
void Encoder_set_callback_increment_fun(callback_increment_fun fun);
void Encoder_set_callback_decrement_fun(callback_decrement_fun fun);

//--------------------------------------------------------------------------------

#endif /* INC_ENCODER_H_ */
