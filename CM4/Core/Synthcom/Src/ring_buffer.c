/*
 * ring_buffer.c
 *
 *  Created on: Apr 1, 2021
 *      Author: karol
 */

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include "ring_buffer.h"

//--------------------------------------------------------------------------------

bool RingBuffer_init(volatile ring_buffer *ring_buffer, volatile uint8_t *data_buffer, size_t data_buffer_size)
{
    assert(ring_buffer);
    assert(data_buffer);
    assert(data_buffer_size > 0);

    if ((ring_buffer) && (data_buffer) && (data_buffer_size > 0))
    {
        ring_buffer->data_buffer = data_buffer;
        ring_buffer->data_buffer_size = data_buffer_size;
//        ring_buffer->head = 0;
//        ring_buffer->tail = 0;
//        ring_buffer->count = 0;
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------

bool RingBuffer_clear(volatile ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        for (size_t i = 0; i < ring_buffer->data_buffer_size; i++)
        {
            ring_buffer->data_buffer[i] = 0;
        }

        ring_buffer->count = 0;
        ring_buffer->head = 0;
        ring_buffer->tail = 0;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------

bool RingBuffer_is_empty(volatile const ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if ((!ring_buffer->count) && (ring_buffer))
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------

size_t RingBuffer_get_len(volatile const ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        return ring_buffer->count;
    }
    return 0;

}

//--------------------------------------------------------------------------------

size_t RingBuffer_get_capacity(volatile const ring_buffer *ring_buffer)
{
    assert(ring_buffer);

    if (ring_buffer)
    {
        return ring_buffer->data_buffer_size;
    }
    return 0;
}

//--------------------------------------------------------------------------------

bool RingBuffer_put_val(volatile ring_buffer *ring_buffer, uint8_t val)
{
    assert(ring_buffer);

    if ((ring_buffer) && (ring_buffer->count != ring_buffer->data_buffer_size))
    {
        ring_buffer->data_buffer[ring_buffer->head] = val;

        ring_buffer->head++;
        ring_buffer->head %= ring_buffer->data_buffer_size;

        ring_buffer->count++;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------

bool RingBuffer_get_val(volatile ring_buffer *ring_buffer, uint8_t *val)
{
    assert(ring_buffer);
    assert(val);

    if ((ring_buffer) && (val) && (ring_buffer->count))
    {
        *val = ring_buffer->data_buffer[ring_buffer->tail];

        ring_buffer->tail++;
        ring_buffer->tail %= ring_buffer->data_buffer_size;

        ring_buffer->count--;
        return true;
    }
    return false;
}
