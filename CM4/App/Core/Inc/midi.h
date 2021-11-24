/*
 * midi.h
 *
 *  Created on: Oct 5, 2021
 *      Author: karol
 */

#ifndef APP_MIDI_H_
#define APP_MIDI_H_

#include "stdint.h"

#define MIDI_NOTE_OFF           0x8
#define MIDI_NOTE_ON            0x9
#define MIDI_POLY_PRESSURE      0xA
#define MIDI_CONTROL_CHANGE     0xB
#define MIDI_PROGRAM_CHANGE     0xC
#define MIDI_CHANNEL_PRESSURE   0xD
#define MIDI_PITCH_BEND         0xE

typedef enum midi_app_states_T
{
    MIDI_APP_IDLE,
    MIDI_APP_START,
    MIDI_APP_RUNNING,
    MIDI_APP_DISCONNECT
} midi_app_states;

void MIDI_App_Process(void);
void MIDI_App_SetState(midi_app_states state);
midi_app_states MIDI_App_GetState(void);

#endif /* APP_MIDI_H_ */
