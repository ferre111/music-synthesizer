#ifndef SYNTHCOM_PACKETS_H_
#define SYNTHCOM_PACKETS_H_

#include <assert.h>

#define MAX_PAYLOAD 256U
#define PAYLOAD {                                           \
                    [SYNTHCOM_MIDI_KEY_ON] = 2U,            \
                    [SYNTHCOM_MIDI_KEY_OFF] = 2U,           \
                    [SYNTHCOM_ENVELOPE_GENERATOR_DATA] = 8U,\
                    [SYNTHCOM_OSCILLATOR_DATA] = 4U         \
                }                                           \

/* Possible packet types */
typedef enum SynthCom_PacketType_T
{
    SynthCom_PacketType_start,

    SYNTHCOM_MIDI_KEY_ON,
    SYNTHCOM_MIDI_KEY_OFF,
    SYNTHCOM_ENVELOPE_GENERATOR_DATA,
    SYNTHCOM_OSCILLATOR_DATA,

    SynthCom_PacketType_end
} SynthCom_PacketType;

typedef struct SynthComPacket_midi_key_on_T
{
    uint8_t note_number;
    uint8_t velocity;
} SynthComPacket_midi_key_on;
/* static assertion to check that the structure is indeed packed */
static_assert(2U == sizeof(SynthComPacket_midi_key_on), "2U != sizeof(SynthComPacket_midi_key_on)");

typedef struct SynthComPacket_midi_key_off_T
{
    uint8_t note_number;
    uint8_t velocity;
} SynthComPacket_midi_key_off;
/* static assertion to check that the structure is indeed packed */
static_assert(2U == sizeof(SynthComPacket_midi_key_off), "2U != sizeof(SynthComPacket_Test)");

typedef struct SynthComPacket_envelope_generator_data_T
{
    uint16_t sustain_level;
    uint16_t attack_time;
    uint16_t decay_time;
    uint16_t release_time;
} SynthComPacket_envelope_generator_data;
/* static assertion to check that the structure is indeed packed */
static_assert(8U == sizeof(SynthComPacket_envelope_generator_data), "8U != sizeof(SynthComPacket_envelope_generator_data)");

typedef struct SynthComPacket_oscillator_data_T
{
    uint8_t oscillator;
    uint8_t activated;
    uint8_t shape;
    uint8_t octave_offset;
} SynthComPacket_oscillator_data;
/* static assertion to check that the structure is indeed packed */
static_assert(4U == sizeof(SynthComPacket_oscillator_data), "4U != sizeof(SynthComPacket_oscillator_data)");

#endif /* AMCOM_PACKETS_H_ */
