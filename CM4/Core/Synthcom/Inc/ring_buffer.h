/*
 * ring_buffer.h
 *
 *  Created on: Apr 1, 2021
 *      Author: karol
 */

#ifndef APP_INC_RING_BUFFER_H_
#define APP_INC_RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//--------------------------------------------------------------------------------

/** @brief Structure describing the ring buffer. */
typedef struct ring_buffer_T
{
    volatile uint8_t *data_buffer;
    size_t data_buffer_size;
    size_t tail;
    size_t head;
    size_t count;
} ring_buffer;

//--------------------------------------------------------------------------------

/**
 * @brief Initializes the given ring buffer structure.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @param *data_buffer pointer to a location in memory, where the ring buffer data will be stored
 * @param data_buffer_size size in bytes of the dataBuffer
 * @return true if all arguments are valid and the ring buffer is initialized successfully, false otherwise
*/
bool RingBuffer_init(volatile ring_buffer *ring_buffer, volatile uint8_t *data_buffer, size_t data_buffer_size);

//--------------------------------------------------------------------------------

/**
 * @brief Clears contents of the given ring buffer.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @return true if the ring buffer is cleared successfully, false otherwise
*/
bool RingBuffer_clear(volatile ring_buffer *ring_buffer);

//--------------------------------------------------------------------------------

/**
 * @brief Checks if the given ring buffer is empty.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @return true if the ring buffer holds no data, false otherwise
*/
bool RingBuffer_is_empty(volatile const ring_buffer *ring_buffer);

//--------------------------------------------------------------------------------

/**
 * @brief Gets the length (in bytes) of the data stored in the given ring buffer.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @return length (in bytes) of the data stored in the ring buffer
*/
size_t RingBuffer_get_len(volatile const ring_buffer *ring_buffer);

//--------------------------------------------------------------------------------

/**
 * @brief Returns the capacity (in bytes) of the given buffer.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @return capacity (in bytes) of the ring buffer (how much values can it store)
*/
size_t RingBuffer_get_capacity(volatile const ring_buffer *ring_buffer);

//--------------------------------------------------------------------------------

/**
 * @brief Appends a single value to the ring buffer. The stored data length will be
 * increased by 1.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @param val value that will be append
 * @return true if the value was added successfully, false otherwise
*/
bool RingBuffer_put_val(volatile ring_buffer *ring_buffer, uint8_t val);

//--------------------------------------------------------------------------------

/**
 * @brief Pulls out a single value from the ring buffer. The stored data length will be
 * decreased by 1.
 *
 * @param *ring_buffer pointer to a \ref ring_buffer structure
 * @param *val pointer to variable where value will be save
 * @return true if the value was pulled out successfully, false otherwise
*/
bool RingBuffer_get_val(volatile ring_buffer *ring_buffer, uint8_t *val);


#endif //_RING_BUFFER_
