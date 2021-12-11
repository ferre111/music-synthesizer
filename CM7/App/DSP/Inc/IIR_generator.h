/*
 * IIR_generator.h
 *
 *  Created on: Nov 26, 2021
 *      Author: karol
 */

#ifndef DSP_INC_IIR_GENERATOR_H_
#define DSP_INC_IIR_GENERATOR_H_

#include "arm_math.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

// if FRACTIONAL macro is equal to 1 then IIR generator operate on fixed point numbers otherwise operate on floating point numbers
#define FRACTIONAL 0U
// if USE_CMSIS_IIR macro is equal to 1 then IIR generator use CMSIS function "arm_biquad_cascade_df1_q31" in IIR_generator_get_next_val function
#define USE_CMSIS_IIR 0U

#define SAMPLING_FREQ 48000U

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef struct IIR_generator_T
{
#if FRACTIONAL
    q31_t val[4];
    q31_t coef[5];
#else
    double coef_a0;
    double coef_a1;
    double val[4];
#endif
} IIR_generator;

/*------------------------------------------------------------------------------------------------------------------------------*/

void IIR_generator_compute_coef(IIR_generator *ctx, double freq);
double IIR_generator_get_next_val(IIR_generator *ctx);

#endif /* DSP_INC_IIR_GENERATOR_H_ */
