/*
 * enveleops_generator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: User
 */

#include <eg.h>
#include "DSP_basic_val.h"

double EG_gen(eg *eg)
{
    double new_val = 0.0;

    switch(eg->status)
    {
        case EG_ATTACK:
            new_val = eg->attack_coef * (double)eg->attack_counter;

            if (++eg->attack_counter == eg->attack_time)
            {
                eg->status = EG_DECAY;
                eg->attack_counter = 0U;
            }
            break;
        case EG_DECAY:
            new_val = (eg->decay_coef * (double)eg->decay_counter + 1.0);

            if (++eg->decay_counter == eg->decay_time)
            {
                eg->status = EG_SUSTAIN;
                eg->decay_counter = 0U;
            }
            break;
        case EG_SUSTAIN:
            new_val = eg->sustain_coef;
            break;
        case EG_RELEASE:
            new_val = (eg->release_coef * (double)eg->release_counter + eg->max_ampl_release_transition);

            if (++eg->release_counter == eg->release_time)
            {
                eg->status = EG_OFF;
                eg->release_counter = 0U;
//                eg->eg_off_fun();
            }
            break;
        case EG_OFF:
            break;
    }
    return new_val;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void EG_set(eg *eg, uint8_t sustain_level, uint32_t attack_time, uint32_t decay_time, uint32_t release_time)
{
    eg->sustain_level = sustain_level;
    eg->attack_time = attack_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg->decay_time = decay_time * NUMBER_OF_SAMPLES_IN_MILISECOND;
    eg->release_time = release_time * NUMBER_OF_SAMPLES_IN_MILISECOND;

    eg->attack_coef = 1.0 / (double)eg->attack_time;
    eg->decay_coef = ((double)eg->sustain_level / 100.0 - 1.0) / (double)eg->decay_time;
    eg->sustain_coef = (double)eg->sustain_level / 100.0;
    eg->max_ampl_release_transition = eg->sustain_coef;
    eg->release_coef = -(double)eg->max_ampl_release_transition / (double)eg->release_time;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void EG_set_eg_off_fun(eg *eg, callback_eg_off_fun off_fun)
{
    eg->eg_off_fun = off_fun;
}

