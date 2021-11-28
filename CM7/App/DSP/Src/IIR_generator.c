/*
 * IIR_generator.c
 *
 *  Created on: Nov 26, 2021
 *      Author: karol
 */

#include <math.h>
#include "arm_math.h"
#include "IIR_generator.h"
#include "sin_gen.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

void IIR_generator_compute_coef(IIR_generator *ctx, double freq)
{
    for (uint8_t i = 0U; i < 4U; i++)
    {
        ctx->val[i] = sin(2.0 * (double)PI * freq * (double)i / (double)SAMPLING_FREQ);
    }

    ctx->coef_a0 = ctx->val[2] / ctx->val[1];
    ctx->coef_a1 = (ctx->val[3] * ctx->val[1] - ctx->val[2] * ctx->val[2]) / (ctx->val[1] * ctx->val[1]);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

double IIR_generator_get_next_val(IIR_generator *ctx)
{
//    ctx->val[0] = ctx->val[1];
    ctx->val[1] = ctx->val[2];
    ctx->val[2] = ctx->val[3];

    ctx->val[3] = ctx->coef_a0 * ctx->val[2] + ctx->coef_a1 * ctx->val[1];
    return ctx->val[3];
}
