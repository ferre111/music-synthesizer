/*
 * envelops_generator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: User
 */

#ifndef DSP_INC_EG_H_
#define DSP_INC_EG_H_

#include "main.h"

typedef void (*callback_eg_off_fun)(void);

typedef enum eg_status_T
{
    EG_ATTACK,
    EG_DECAY,
    EG_SUSTAIN,
    EG_RELEASE,
    EG_OFF
} eg_status;

/*
 *            /|\
 *           / | \
 *          /  |  \
 *         /   |   \
 *        /    |    \
 *       /     |     \________________  Sustain level
 *      /      |     |                |\
 *     /       |     |                | \
 *    /        |     |                |  \
 * __/         |     |                |   \__
 *     Attack             Sustain
 *              Decay                  Release
 */
typedef struct eg_T
{
    /* sustain level, 100 - 100%, 50 - 50% */
    uint8_t sustain_level;
    /* time as a 1/480000s */
    uint32_t attack_time;
    uint32_t decay_time;
    uint32_t release_time;
    /* coefficients */
    double attack_coef;
    double decay_coef;
    double sustain_coef;
    double release_coef;
    /* max amplitude during transition to Release state, this maximal amplitude value results from a linear equation related with ADSR envelope generator */
    double max_ampl_release_transition;

    uint32_t attack_counter;
    uint32_t decay_counter;
    uint32_t release_counter;

    /* function call when eg goes to state EG_OFF*/
    callback_eg_off_fun eg_off_fun;

    eg_status status;
} eg;

double EG_gen(eg *eg);
void EG_set(eg *eg, uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time);
void EG_set_max_ampl_release_transition(eg *eg, double max_ampl_release_transition);
void EG_set_eg_off_fun(eg *eg, callback_eg_off_fun off_fun);
void EG_set_status(eg *eg, eg_status status);

#endif /* DSP_INC_EG_H_ */
