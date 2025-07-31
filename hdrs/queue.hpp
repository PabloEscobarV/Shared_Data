/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 07:56:22 by BlackRider        #+#    #+#             */
/*   Updated: 2025/07/31 15:00:05 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstdint>

/*********************************************************************************************************************
 *  @class FSQueue
 *  @brief Fixed-size circular queue template class for embedded systems.
 *
 *  This template class implements a circular buffer queue with fixed capacity determined at compile time.
 *  It provides standard queue operations (push, pop, peek) plus additional functionality for priority
 *  reordering through the swap method. The implementation is designed for embedded systems where
 *  dynamic memory allocation should be avoided and memory usage must be predictable.
 *
 *  @tparam data_t Type of data stored in the queue
 *  @tparam size Maximum number of elements the queue can hold (1-255)
 ********************************************************************************************************************/

template <typename data_t, uint8_t size>
class FSQueue
{
  public:
    /*******************************************************************************************************************
     *  @brief Default constructor - initializes empty queue.
     *
     *  Constructs an empty queue with head, tail, and count set to zero.
     *******************************************************************************************************************/
    FSQueue() : head(0), tail(0), count(0) {}

    /*******************************************************************************************************************
     *  @brief Add element to the back of the queue.
     *
     *  Pushes a new element to the tail of the queue if space is available.
     *  The queue operates in FIFO (First In, First Out) mode.
     *
     *  @param item Const reference to the item to be added.
     *  @return true if item was successfully added, false if queue is full.
     *******************************************************************************************************************/
    bool push(const data_t &item);

    /*******************************************************************************************************************
     *  @brief Remove element from the front of the queue without returning value.
     *
     *  Removes the element at the head of the queue but does not return its value.
     *  This is useful when you only need to discard the front element.
     *
     *  @return true if element was successfully removed, false if queue is empty.
     *******************************************************************************************************************/
    bool pop();

    /*******************************************************************************************************************
     *  @brief Remove element from the front of the queue and return its value.
     *
     *  Removes the element at the head of the queue and copies its value to the
     *  provided reference parameter.
     *
     *  @param item Reference to store the popped element value.
     *  @return true if element was successfully removed, false if queue is empty.
     *******************************************************************************************************************/
    bool pop(data_t &item);

    /*******************************************************************************************************************
     *  @brief Reorder queue by moving head element to specified position.
     *
     *  This function provides priority reordering by moving the current head element
     *  to the specified position within the queue. All elements between the head and
     *  target position are shifted forward by one position.
     *
     *  @param swap_item Target position (0-based) where head element should be moved.
     *                   If >= count, element is moved to the last position.
     *******************************************************************************************************************/
    void swap(uint16_t swap_item);

    /*******************************************************************************************************************
     *  @brief Peek at the front element without removing it.
     *
     *  Returns a copy of the element at the head of the queue without modifying
     *  the queue state. This allows inspection of the next element to be popped.
     *
     *  @param item Reference to store the peeked element value.
     *  @return true if element was successfully retrieved, false if queue is empty.
     *******************************************************************************************************************/
    bool peek(data_t &item) const;

    /*******************************************************************************************************************
     *  @brief Get current number of elements in the queue.
     *
     *  @return Number of elements currently stored in the queue (0 to size).
     *******************************************************************************************************************/
    uint8_t get_count() const;

    /*******************************************************************************************************************
     *  @brief Check if the queue is empty.
     *
     *  @return true if queue contains no elements, false otherwise.
     *******************************************************************************************************************/
    bool is_empty() const;

    /*******************************************************************************************************************
     *  @brief Check if the queue is full.
     *
     *  @return true if queue has reached maximum capacity, false otherwise.
     *******************************************************************************************************************/
    bool is_full() const;

   private:
    uint8_t head;   ///< Index of the first element (next to be popped)
    uint8_t tail;   ///< Index where next element will be pushed
    uint8_t count;  ///< Current number of elements in the queue
    data_t  data[size]; ///< Fixed-size array buffer for queue elements

    /*******************************************************************************************************************
     *  @brief Increment queue parameter with wraparound.
     *
     *  Helper function to increment head or tail pointer with automatic wraparound
     *  when reaching the end of the buffer array.
     *
     *  @param param Reference to head or tail parameter to increment.
     *******************************************************************************************************************/
    inline void incr_queue_param(uint8_t& param) { param = static_cast<uint8_t>((param + 1u) % size); }

    /*******************************************************************************************************************
     *  @brief Get queue array index with wraparound.
     *
     *  Helper function to calculate array index with wraparound for accessing
     *  queue elements at relative positions.
     *
     *  @param i Index to wrap around.
     *  @return Wrapped index within array bounds.
     *******************************************************************************************************************/
    inline uint8_t get_queue_idx(uint8_t i) { return i % size; }
};

/* ===================================================================================================================
 *  Method implementations
 * ================================================================================================================ */

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::push(const data_t &item)
{
  bool result = false;

  if (!is_full())
  {
    data[tail] = item;
    incr_queue_param(tail);
    ++count;
    result = true;
  }
  return result;
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::pop(data_t &item)
{
  bool result = false;

  if (!is_empty())
  {
    item = data[head];
    incr_queue_param(head);
    --count;
    result = true;
  }
  return result;
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::pop()
{
  bool result = false;

  if (!is_empty())
  {
    incr_queue_param(head);
    --count;
    result = true;
  }
  return result;
}

template <typename data_t, uint8_t size>
void FSQueue<data_t, size>::swap(uint16_t swap_item)
{
  data_t temp = data[head];

  if (!is_empty() && swap_item)
  {
    if (swap_item >= count)
    {
      swap_item = count - 1; // Adjust to the last valid index
    }
    for (int i = 0; i < swap_item; ++i)
    {
      data[get_queue_idx(static_cast<uint8_t>(i + head))] = data[get_queue_idx(static_cast<uint8_t>(i + head + 1))];
    }
    data[get_queue_idx(static_cast<uint8_t>(swap_item + head))] = temp;
  }
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::peek(data_t& item) const
{
  bool result = false;

  if (!is_empty())
  {
    item = data[head];
    result = true;
  }
  return result;
}

template <typename data_t, uint8_t size>
uint8_t FSQueue<data_t, size>::get_count() const
{
  return count;
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::is_empty() const
{
  return (count == 0);
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::is_full() const
{
  return (count == size);
}

#endif // QUEUE_HPP