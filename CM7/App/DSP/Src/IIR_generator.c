/*
 * IIR_generator.c
 *
 *  Created on: Nov 26, 2021
 *      Author: karol
 */

#include <math.h>
#include <synth.h>
#include "arm_math.h"
#include "IIR_generator.h"
#include "utility.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#if FRACTIONAL && USE_CMSIS_IIR
static arm_biquad_casd_df1_inst_q31 arm_biquad_q31;
#endif
/*------------------------------------------------------------------------------------------------------------------------------*/

void IIR_generator_compute_coef(IIR_generator *ctx, double freq)
{
#if FRACTIONAL
    float xf;
    q31_t x;

#if USE_CMSIS_IIR
    arm_biquad_cascade_df1_init_q31(&arm_biquad_q31, 1U, ctx->coef, ctx->val, 1U);
#endif

    for (uint8_t i = 0U; i < 4U; i++)
    {
        xf = (float)freq * (float)i / (float)SAMPLING_FREQ;
        arm_float_to_q31(&xf, &x, 1U);
        ctx->val[i] = -arm_sin_q31(x);
    }

    /* variable aux is used to scale IIR coefficient, without it coefficient exceed range */
    xf = 0.5;
    q31_t aux;
    arm_float_to_q31(&xf, &aux, 1U);

    /* the rest of the coefficients are equal to zero */
    ctx->coef[3] = Q31_DIVISION(Q31_MULTIPLICATION(ctx->val[2], aux), ctx->val[1]);
    ctx->coef[4] = Q31_MULTIPLICATION(Q31_DIVISION((Q31_MULTIPLICATION(ctx->val[3], ctx->val[1]) - Q31_MULTIPLICATION(ctx->val[2], ctx->val[2])),
            (Q31_MULTIPLICATION(ctx->val[1], ctx->val[1]))), aux);

#else
    for (uint8_t i = 0U; i < 4U; i++)
        {
            ctx->val[i] = sin(2.0 * (double)PI * freq * (double)i / (double)SAMPLING_FREQ);
        }

        ctx->coef_a0 = ctx->val[2] / ctx->val[1];
        ctx->coef_a1 = (ctx->val[3] * ctx->val[1] - ctx->val[2] * ctx->val[2]) / (ctx->val[1] * ctx->val[1]);
#endif
}

/*------------------------------------------------------------------------------------------------------------------------------*/

double IIR_generator_get_next_val(IIR_generator *ctx)
{
#if FRACTIONAL
#if USE_CMSIS_IIR
    q31_t tmp = 0;
    arm_biquad_cascade_df1_q31(&arm_biquad_q31, &tmp, &ctx->val[3], 1U);
#else
    ctx->val[1] = ctx->val[2];
    ctx->val[2] = ctx->val[3];
    ctx->val[3] = (Q31_MULTIPLICATION(ctx->coef[3], ctx->val[2]) + Q31_MULTIPLICATION(ctx->coef[4], ctx->val[1])) << 1;
#endif
    return (double)((float32_t)ctx->val[3] / 2147483648U);
#else
    ctx->val[1] = ctx->val[2];
    ctx->val[2] = ctx->val[3];

    ctx->val[3] = ctx->coef_a0 * ctx->val[2] + ctx->coef_a1 * ctx->val[1];
    return ctx->val[3];
#endif
}
