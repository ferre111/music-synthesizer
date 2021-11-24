/*
 * midi.c
 *
 *  Created on: Oct 5, 2021
 *      Author: karol
 */

#include "midi.h"
#include "usbh_MIDI.h"
#include "usb_host.h"
#include "stdbool.h"
#include "synthcom.h"

extern USBH_HandleTypeDef hUsbHostFS;

/*------------------------------------------------------------------------------------------------------------------------------*/

/* USB MIDI buffer : max received data 16 bytes */
#define RX_BUFF_SIZE 16

/*------------------------------------------------------------------------------------------------------------------------------*/

typedef struct midi_ctx_T
{
    midi_app_states midi_state;
    uint32_t        channel;
    uint8_t*        midi_buffer_recive;
    uint8_t*        midi_buffer_read;
}midi_ctx;

/*------------------------------------------------------------------------------------------------------------------------------*/

/* MIDI reception buffer */
static uint8_t MIDI_RX_Buffer[2][RX_BUFF_SIZE];

static midi_ctx ctx = {.midi_state = MIDI_APP_IDLE, .channel = 0U, .midi_buffer_recive = MIDI_RX_Buffer[0], .midi_buffer_read = MIDI_RX_Buffer[1]};

/*------------------------------------------------------------------------------------------------------------------------------*/

static void MIDI_App_ReceiveFrame(void);
static void swap_buffers(void);

/*------------------------------------------------------------------------------------------------------------------------------*/

void MIDI_App_Process(void)
{
    switch(ctx.midi_state)
    {
    case MIDI_APP_IDLE:
        break;
    case MIDI_APP_START:
        /* start first reception */
        USBH_MIDI_Receive(&hUsbHostFS, ctx.midi_buffer_recive, RX_BUFF_SIZE);
        MIDI_App_SetState(MIDI_APP_RUNNING);
        break;
    case MIDI_APP_RUNNING:
        break;
    case MIDI_APP_DISCONNECT:
        USBH_MIDI_Stop(&hUsbHostFS);
        MIDI_App_SetState(MIDI_APP_IDLE);
        break;
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void MIDI_App_SetState(midi_app_states state)
{
    ctx.midi_state = state;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

midi_app_states MIDI_App_GetState(void)
{
    return ctx.midi_state;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    /* start a new reception */
    swap_buffers();
    USBH_MIDI_Receive(&hUsbHostFS, ctx.midi_buffer_recive, RX_BUFF_SIZE); //
    MIDI_App_ReceiveFrame();
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void MIDI_App_ReceiveFrame(void)
{
    if (ctx.channel == (ctx.midi_buffer_read[1] & ~(0xF0U)))   //check MIDI channel
    {
        switch (ctx.midi_buffer_read[1] >> 4U)                //check status
        {
        case MIDI_NOTE_OFF:
            SynthCom_transmit(SYNTHCOM_MIDI_KEY_OFF, &ctx.midi_buffer_read[2]);
            break;
        case MIDI_NOTE_ON:
            SynthCom_transmit(SYNTHCOM_MIDI_KEY_ON, &ctx.midi_buffer_read[2]);
            break;
        case MIDI_POLY_PRESSURE:
            break;
        case MIDI_CONTROL_CHANGE:
            break;
        case MIDI_PROGRAM_CHANGE:
            break;
        case MIDI_CHANNEL_PRESSURE:
            break;
        case MIDI_PITCH_BEND:
            break;
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/

static void swap_buffers(void)
{
    static bool flag;

    /* change flag value */
    flag ^= 1U;

    if (flag)
    {
        ctx.midi_buffer_recive = MIDI_RX_Buffer[1];
        ctx.midi_buffer_read = MIDI_RX_Buffer[0];
    }
    else
    {
        ctx.midi_buffer_recive = MIDI_RX_Buffer[0];
        ctx.midi_buffer_read = MIDI_RX_Buffer[1];
    }
}
