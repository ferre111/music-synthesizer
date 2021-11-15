#include "main.h"
#include "synthcom.h"
#include "ring_buffer.h"
#include <stdlib.h>

SynthComPacket_Test test; //todo

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
static uint8_t data_buffer_CM7_to_CM4[DATA_BUFFER_SIZE] __attribute((section(".common_buffers")));
static uint8_t data_buffer_CM4_to_CM7[DATA_BUFFER_SIZE] __attribute((section(".common_buffers")));
/* ring buffer structures */
static ring_buffer ring_buffer_CM7_to_CM4 __attribute((section(".common_buffers")));;
static ring_buffer ring_buffer_CM4_to_CM7 __attribute((section(".common_buffers")));;
/* message counters */
static uint32_t message_counter_CM7 __attribute((section(".common_buffers")));
static uint32_t message_counter_CM4 __attribute((section(".common_buffers")));
/* temporary buffer required during receiving data */
static uint8_t tmp_buffer[MAX_PAYLOAD];

/* context for Cortex M7*/
#ifdef CORE_CM7
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM7_to_CM4, .ring_buffer_Rx = &ring_buffer_CM4_to_CM7, .message_counter_own = &message_counter_CM7, .message_counter_second_core = &message_counter_CM4};
#endif

/* context for Cortex M4 */
#ifdef CORE_CM4
static synthcom_ctx ctx = {.ring_buffer_Tx = &ring_buffer_CM4_to_CM7, .ring_buffer_Rx = &ring_buffer_CM7_to_CM4, .message_counter_own = &message_counter_CM4, .message_counter_second_core = &message_counter_CM7};
#endif

/*------------------------------------------------------------------------------------------------------------------------------*/

void SynthCom_Init(void)
{
    /* Init ring buffers */
    RingBuffer_init(ctx.ring_buffer_Rx, data_buffer_CM7_to_CM4, DATA_BUFFER_SIZE);
    RingBuffer_init(ctx.ring_buffer_Tx, data_buffer_CM4_to_CM7, DATA_BUFFER_SIZE);
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void SynthCom_transmit(SynthCom_PacketType packet_type, void* data)
{
    /* first write packet type byte */
    RingBuffer_put_val(ctx.ring_buffer_Tx, (uint8_t)packet_type);

    for (size_t i = 0; i < payload_packets[packet_type]; i++)
    {
        /* write the rest of the data */
        RingBuffer_put_val(ctx.ring_buffer_Tx, *((uint8_t*)data++));
    }

    /* increment second core message counter */
    (*ctx.message_counter_second_core)++;
}

/*------------------------------------------------------------------------------------------------------------------------------*/

void SynthCom_receive(void)
{
    SynthCom_PacketType packet_type = SynthCom_PacketType_end;
    bool status = false;

    if (0U != *ctx.message_counter_own)
    {
        /* Get packet type byte */
        RingBuffer_get_val(ctx.ring_buffer_Rx, &packet_type);

        /* Get the rest of the data */
        for (size_t i = 0; i < payload_packets[packet_type]; i++)
        {
            RingBuffer_get_val(ctx.ring_buffer_Rx, (tmp_buffer + i));
        }

        switch (packet_type)
        {
        case SYNTHCOM_TEST:
            test.angle = ((SynthComPacket_Test *)tmp_buffer)->angle;
            //test.buff =
            test.data_x = ((SynthComPacket_Test *)tmp_buffer)->data_x;
            test.data_y = ((SynthComPacket_Test *)tmp_buffer)->data_y;
            break;
        default:
            //todo error
            break;
        }

        /* decrement second core message counter */
        (*ctx.message_counter_own)--;
    }
}

/*------------------------------------------------------------------------------------------------------------------------------*/
