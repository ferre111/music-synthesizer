#include "main.h"
#include "synthcom.h"
#include "ring_buffer.h"
#include "utility.h"
#include "sin_gen.h"

/*------------------------------------------------------------------------------------------------------------------------------*/

/* Size of data buffer use by ring buffer */
#define DATA_BUFFER_SIZE 1024

#define MESSAGE_COUNTER_CM7_ADD (uint32_t*)0x3800FFF8
#define MESSAGE_COUNTER_CM4_ADD (uint32_t*)0x3800FFFC

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
static uint8_t data_buffer_CM7_to_CM4[DATA_BUFFER_SIZE] __attribute((section(".common_buffers")));
static uint8_t data_buffer_CM4_to_CM7[DATA_BUFFER_SIZE] __attribute((section(".common_buffers")));
/* ring buffer structures */
static ring_buffer ring_buffer_CM7_to_CM4 __attribute((section(".common_buffers")));;
static ring_buffer ring_buffer_CM4_to_CM7 __attribute((section(".common_buffers")));;
/* temporary buffer required during receiving data */
static uint8_t tmp_buffer[MAX_PAYLOAD];

/* context for Cortex M7*/
#ifdef CORE_CM7
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM7_to_CM4, .ring_buffer_Rx = &ring_buffer_CM4_to_CM7};
#endif

/* context for Cortex M4 */
#ifdef CORE_CM4
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM4_to_CM7, .ring_buffer_Rx = &ring_buffer_CM7_to_CM4};
#endif

/*------------------------------------------------------------------------------------------------------------------------------*/

void SynthCom_Init(void)
{
    /* Init ring buffers */
    RingBuffer_init(ctx.ring_buffer_Tx, data_buffer_CM7_to_CM4, DATA_BUFFER_SIZE);
    RingBuffer_init(ctx.ring_buffer_Rx, data_buffer_CM4_to_CM7, DATA_BUFFER_SIZE);

    /* There was a problem when variables was used instead of pointers due to allocation after -ofast flag was set
     * (different address for CM7 and CM4). This is simple workaround.
     */
    ctx.message_counter_own = MESSAGE_COUNTER_CM7_ADD;
    *ctx.message_counter_own = 0;
    ctx.message_counter_second_core = MESSAGE_COUNTER_CM4_ADD;
    *ctx.message_counter_second_core = 0;
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
        case SYNTHCOM_MIDI_KEY_ON:
            sin_gen_set_play(true, ((SynthComPacket_midi_key_on *)tmp_buffer)->note_number);
            break;
        case SYNTHCOM_MIDI_KEY_OFF:
            sin_gen_set_play(false, ((SynthComPacket_midi_key_off *)tmp_buffer)->note_number);
            break;
        default:
            utility_ErrLedOn();
            goto ret;
            break;
        }

        (*ctx.message_counter_own)--;
    }
    status = true;
ret:
    return status;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
