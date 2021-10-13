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
#include "sin_gen.h"

#define RX_BUFF_SIZE 16 /* USB MIDI buffer : max received data 16 bytes */

extern USBH_HandleTypeDef hUsbHostFS;

uint8_t MIDI_RX_Buffer[RX_BUFF_SIZE]; // MIDI reception buffer

static midi_ctx ctx = {.midi_state = MIDI_APP_IDLE, .channel = 0};
static void MIDI_App_ReceiveFrame(void);

void MIDI_App_Process(void)
{
    switch(ctx.midi_state)
    {
    case MIDI_APP_IDLE:
        break;
    case MIDI_APP_START:
        USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // start first reception
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

void MIDI_App_SetState(midi_app_states state)
{
    ctx.midi_state = state;
}

midi_app_states MIDI_App_GetState(void)
{
    return ctx.midi_state;
}

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // start a new reception
    MIDI_App_ReceiveFrame();
}

static void MIDI_App_ReceiveFrame(void)
{
    if (ctx.channel == (MIDI_RX_Buffer[1] & ~(0xF0U)))   //check MIDI channel
    {
        switch (MIDI_RX_Buffer[1] >> 4U)                //check status
        {
        case MIDI_NOTE_OFF:
            sin_gen_set_play(false, MIDI_RX_Buffer[2]);
            break;
        case MIDI_NOTE_ON:
            sin_gen_set_play(true, MIDI_RX_Buffer[2]);
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
