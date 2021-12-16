/*
 * wavetable.c
 *
 *  Created on: Jan 25, 2021
 *      Author: Karol Witusik
 // */
#include <string.h>
#include <stdio.h>
#include "wavetable.h"
#include "DSP_basic_val.h"
#include "i2s.h"
#include "utility.h"
#include "ext_flash_driver.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

#define EXT_FLASH_BASE_ADDRESS 0x90000000U

/*------------------------------------------------------------------------------------------------------------------------------*/

int16_t wavetable_ram[OSCILLATOR_COUNTS][SAMPLE_COUNT] = {0};

/*------------------------------------------------------------------------------------------------------------------------------*/

static uint32_t wavetable_shape_offset[Wavetable_shape_end] =
{
        [WAVETABLE_SHAPE_SINE]      = 0U,
        [WAVETABLE_SHAPE_SQUARE]    = SAMPLE_COUNT * 2U,
        [WAVETABLE_SHAPE_TRIANGLE]  = SAMPLE_COUNT * 4U,
        [WAVETABLE_SHAPE_SAW]       = SAMPLE_COUNT * 6U
};

/*------------------------------------------------------------------------------------------------------------------------------*/

void Wavetable_init(void)
{
    QSPI_MemoryMappedTypeDef tmp = { .TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE, .TimeOutPeriod = 0xFFFF };
    HAL_QSPI_MemoryMapped(&hqspi, &cmd_FRQIO, &tmp);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Wavetable_load_new_wavetable(synth_oscillator *oscillator)
{
    uint8_t count_currently_enabled_osc = 0U;
    uint32_t basic_address = 0U;

    /* at first we need to know how many oscillators are currently working */
    for (uint8_t osc = 0U; osc < OSCILLATOR_COUNTS; osc++)
    {
        if (oscillator[osc].activated)
        {
            count_currently_enabled_osc++;
        }
    }

    utility_LoadLedOn();

    __disable_irq();
    HAL_I2S_DMAPause(&hi2s1);
    __HAL_I2S_DISABLE(&hi2s1);

    for (uint8_t osc = 0U; osc < OSCILLATOR_COUNTS; osc++)
    {
        if (oscillator[osc].activated)
        {
            basic_address = EXT_FLASH_BASE_ADDRESS + wavetable_shape_offset[oscillator[osc].shape];

            for (uint32_t sample = 0U; sample < SAMPLE_COUNT; sample++)
            {
                wavetable_ram[osc][sample] = (*(int16_t*)(basic_address + sample * 2U)) / (int16_t)(count_currently_enabled_osc * VOICE_COUNT);
            }
        }
    }

    __HAL_I2S_ENABLE(&hi2s1);
    HAL_I2S_DMAResume(&hi2s1);
    __enable_irq();

    utility_LoadLedOff();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void Wavetable_load_new_wavetable_fm(synth_oscillators_fm *fm_oscillator)
{
    uint32_t basic_address = 0U;

    utility_LoadLedOn();

    __disable_irq();
    HAL_I2S_DMAPause(&hi2s1);
    __HAL_I2S_DISABLE(&hi2s1);

    basic_address = EXT_FLASH_BASE_ADDRESS + wavetable_shape_offset[fm_oscillator->shape_carrier_osc];

    for (uint32_t sample = 0U; sample < SAMPLE_COUNT; sample++)
    {
        wavetable_ram[FIRST_OSC][sample] = (*(int16_t*)(basic_address + sample * 2U)) / (int16_t)VOICE_COUNT;
    }

    basic_address = EXT_FLASH_BASE_ADDRESS + wavetable_shape_offset[fm_oscillator->shape_modulator_osc];

    for (uint32_t sample = 0U; sample < SAMPLE_COUNT; sample++)
    {
        wavetable_ram[SECOND_OSC][sample] = (*(int16_t*)(basic_address + sample * 2U)) / (int16_t)VOICE_COUNT;
    }

    __HAL_I2S_ENABLE(&hi2s1);
    HAL_I2S_DMAResume(&hi2s1);
    __enable_irq();

    utility_LoadLedOff();
}
