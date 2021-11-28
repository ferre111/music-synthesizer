/*
 * IIR_generator.h
 *
 *  Created on: Nov 26, 2021
 *      Author: karol
 */

#ifndef DSP_INC_IIR_GENERATOR_H_
#define DSP_INC_IIR_GENERATOR_H_

/*------------------------------------------------------------------------------------------------------------------------------*/

#define SAMPLING_FREQ 48000U

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef struct IIR_generator_T
{
    double coef_a0;
    double coef_a1;
    double val[4];
} IIR_generator;

/*------------------------------------------------------------------------------------------------------------------------------*/

void IIR_generator_compute_coef(IIR_generator *ctx, double freq);
double IIR_generator_get_next_val(IIR_generator *ctx);

#endif /* DSP_INC_IIR_GENERATOR_H_ */
