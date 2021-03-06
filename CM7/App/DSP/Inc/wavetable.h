/*
 * wavetable.h
 *
 *  Created on: Jan 25, 2021
 *      Author: Karol Witusik
 */

#ifndef __WAVETABLE_H
#define __WAVETABLE_H

#include <synth.h>
#include "main.h"
#include "DSP_basic_val.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

int16_t wavetable_ram[OSCILLATOR_COUNTS][SAMPLE_COUNT];

void Wavetable_init(void);
void Wavetable_load_new_wavetable(synth_oscillator *osc);
void Wavetable_load_new_wavetable_fm(synth_oscillators_fm *fm_oscillator);
#endif
