/*
 * wavetable.h
 *
 *  Created on: Jan 25, 2021
 *      Author: Karol Witusik
 */

#pragma once

#include "main.h"

#define BASIC_FREQ 1
#define SAMPLE_COUNT 48000

const int16_t wavetable_sin[SAMPLE_COUNT];
const int16_t wavetable_tri[SAMPLE_COUNT];
const int16_t wavetable_sqr[SAMPLE_COUNT];
int16_t *wavetable;

void button_click(void);
