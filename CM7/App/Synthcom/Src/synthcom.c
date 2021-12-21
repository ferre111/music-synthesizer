#include <synth.h>
#include "main.h"
#include "synthcom.h"
#include "ring_buffer.h"
#include "utility.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Size of data buffer use by ring buffer */
#define DATA_BUFFER_SIZE 1024

/*------------------------------------------------------------------------------------------------------------------------------*/
/* structure with pointer necessary during transmitting/receiving data */
typedef struct synthcom_ctx_T
{
    volatile ring_buffer *ring_buffer_Tx;
    volatile ring_buffer *ring_buffer_Rx;
    /* counter with message count for core handling this structure */
    volatile uint32_t *message_counter_own;
    /* counter with message count for second core */
    volatile uint32_t *message_counter_second_core;
} synthcom_ctx;

/*------------------------------------------------------------------------------------------------------------------------------*/

/* array with payload for all message define in synthcom_packets.h file */
static uint32_t payload_packets[SynthCom_PacketType_end] = PAYLOAD;

/* data buffer use by ring buffer */
static uint8_t data_buffer_CM7_to_CM4[DATA_BUFFER_SIZE] __attribute((section(".buffer_CM7_to_CM4")));
static uint8_t data_buffer_CM4_to_CM7[DATA_BUFFER_SIZE] __attribute((section(".buffer_CM4_to_CM7")));
/* ring buffer structures */
static ring_buffer ring_buffer_CM7_to_CM4 __attribute((section(".ring_buffer_CM7_to_CM4")));
static ring_buffer ring_buffer_CM4_to_CM7 __attribute((section(".ring_buffer_CM4_to_CM7")));
/* message counters */
static uint32_t message_counter_CM7_to_CM4 __attribute((section(".message_counter_CM7_to_CM4")));
static uint32_t message_counter_CM4_to_CM7 __attribute((section(".message_counter_CM4_to_CM7")));
/* temporary buffer required during receiving data */
static uint8_t tmp_buffer[MAX_PAYLOAD];

/* context for Cortex M7*/
#ifdef CORE_CM7
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM7_to_CM4, .ring_buffer_Rx = &ring_buffer_CM4_to_CM7, .message_counter_own = &message_counter_CM4_to_CM7, .message_counter_second_core = &message_counter_CM7_to_CM4};
#endif

/* context for Cortex M4 */
#ifdef CORE_CM4
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM4_to_CM7, .ring_buffer_Rx = &ring_buffer_CM7_to_CM4, .message_counter_own = &message_counter_CM7_to_CM4, .message_counter_second_core = &message_counter_CM4_to_CM7};
#endif

/*------------------------------------------------------------------------------------------------------------------------------*/

void SynthCom_Init(void)
{
#ifdef CORE_CM7
    /* Init ring buffers */
    RingBuffer_init(ctx.ring_buffer_Tx, data_buffer_CM7_to_CM4, DATA_BUFFER_SIZE);
    RingBuffer_init(ctx.ring_buffer_Rx, data_buffer_CM4_to_CM7, DATA_BUFFER_SIZE);
    message_counter_CM7_to_CM4 = 0;
#endif

#ifdef CORE_CM4
    /* Init ring buffers */
    RingBuffer_init(ctx.ring_buffer_Rx, data_buffer_CM7_to_CM4, DATA_BUFFER_SIZE);
    RingBuffer_init(ctx.ring_buffer_Tx, data_buffer_CM4_to_CM7, DATA_BUFFER_SIZE);
#endif
}

/*------------------------------------------------------------------------------------------------------------------------------*/

bool SynthCom_transmit(SynthCom_PacketType packet_type, void* data)
{
    bool status = false;
    /* first write packet type byte */
    status = RingBuffer_put_val(ctx.ring_buffer_Tx, (uint8_t)packet_type);
    if (false == status)
    {
        utility_ErrLedOn();
        goto ret;
    }

    for (size_t i = 0; i < payload_packets[packet_type]; i++)
    {
        /* write the rest of the data */
        status = RingBuffer_put_val(ctx.ring_buffer_Tx, *((uint8_t*)data++));
        if (false == status)
        {
            utility_ErrLedOn();
            goto ret;
        }
    }

    /* increment second core message counter */
    (*ctx.message_counter_second_core)++;

    status = true;
ret:
    return status;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

bool SynthCom_process(void)
{
    bool status = false;
    SynthCom_PacketType packet_type = SynthCom_PacketType_end;

    if (0U != *ctx.message_counter_own)
    {
        /* Get packet type byte */
        status = RingBuffer_get_val(ctx.ring_buffer_Rx, &packet_type);
        if (false == status)
        {
            utility_ErrLedOn();
            goto ret;
        }

        /* Get the rest of the data */
        for (size_t i = 0; i < payload_packets[packet_type]; i++)
        {
            status = RingBuffer_get_val(ctx.ring_buffer_Rx, (tmp_buffer + i));
            if (false == status)
            {
                utility_ErrLedOn();
                goto ret;
            }
        }

        switch (packet_type)
        {
#ifdef CORE_CM7
        case SYNTHCOM_MIDI_KEY_ON:
            Synth_set_voice_start_play(((SynthComPacket_midi_key_on *)tmp_buffer)->note_number, ((SynthComPacket_midi_key_on *)tmp_buffer)->velocity);
            break;
        case SYNTHCOM_MIDI_KEY_OFF:
            Synth_set_voice_stop_play(((SynthComPacket_midi_key_off *)tmp_buffer)->note_number);
            break;
        case SYNTHCOM_MIDI_PITCH_BEND:
            Synth_pitch_bend_change(((SynthComPacket_midi_pitch_bend *)tmp_buffer)->pitch_bend);
            break;
        case SYNTHCOM_ENVELOPE_GENERATOR_DATA:
            Synth_set_ampl_envelop_generator(((SynthComPacket_envelope_generator_data *)tmp_buffer)->sustain_level, ((SynthComPacket_envelope_generator_data *)tmp_buffer)->attack_time,
                    ((SynthComPacket_envelope_generator_data *)tmp_buffer)->decay_time, ((SynthComPacket_envelope_generator_data *)tmp_buffer)->release_time);
            break;
        case SYNTHCOM_FIRST_OSCILLATOR_DATA:
            Synth_set_oscillator(((SynthComPacket_first_oscillator_data *)tmp_buffer)->oscillator, ((SynthComPacket_first_oscillator_data *)tmp_buffer)->activated,
                    ((SynthComPacket_first_oscillator_data *)tmp_buffer)->shape, ((SynthComPacket_first_oscillator_data *)tmp_buffer)->octave_offset,
                    0U, ((SynthComPacket_first_oscillator_data *)tmp_buffer)->volume);
            break;
        case SYNTHCOM_OTHER_OSCILLATOR_DATA:
            Synth_set_oscillator(((SynthComPacket_other_oscillator_data *)tmp_buffer)->oscillator, ((SynthComPacket_other_oscillator_data *)tmp_buffer)->activated,
                                ((SynthComPacket_other_oscillator_data *)tmp_buffer)->shape, ((SynthComPacket_other_oscillator_data *)tmp_buffer)->octave_offset,
                                ((SynthComPacket_other_oscillator_data *)tmp_buffer)->phase, ((SynthComPacket_other_oscillator_data *)tmp_buffer)->volume);
            break;
        case SYNTHCOM_FM_SYNTHESIS_DATA:
            Synth_set_FM_oscillator(((SynthComPacket_fm_synthesis_data *)tmp_buffer)->carrier_shape, ((SynthComPacket_fm_synthesis_data *)tmp_buffer)->modulator_shape,
                                    ((SynthComPacket_fm_synthesis_data *)tmp_buffer)->mod_index, ((SynthComPacket_fm_synthesis_data *)tmp_buffer)->freq_mode,
                                    ((SynthComPacket_fm_synthesis_data *)tmp_buffer)->freq, ((SynthComPacket_fm_synthesis_data *)tmp_buffer)->volume);
            break;
#endif
#ifdef CORE_CM4
        case SYNTHCOM_MIDI_KEY_ON:
        case SYNTHCOM_MIDI_KEY_OFF:
        case SYNTHCOM_MIDI_PITCH_BEND:
        case SYNTHCOM_ENVELOPE_GENERATOR_DATA:
        case SYNTHCOM_FIRST_OSCILLATOR_DATA:
        case SYNTHCOM_OTHER_OSCILLATOR_DATA:
        case SYNTHCOM_FM_SYNTHESIS_DATA:
#endif
        default:
            utility_ErrLedOn();
            goto ret;
            break;
        }

        /* decrement second core message counter */
        (*ctx.message_counter_own)--;
    }
    status = true;
ret:
    return status;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
