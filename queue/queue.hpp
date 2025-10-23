/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 20:49:05 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/23 21:13:39 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstdint>

template <typename data_t, uint16_t size>
class FSQueue
{
    uint16_t  head;
    uint16_t  tail;
    uint16_t  count;
    data_t    data[size];

    inline void incr_queue_param(uint16_t& param) { param = static_cast<uint16_t>((param + 1u) % size); }
    inline uint16_t get_queue_idx(uint16_t i) { return i % size; }
  public:
    FSQueue() : head(0), tail(0), count(0) {}
    bool push(const data_t &item, const uint16_t position = 0);
    bool remove();
    bool pop(data_t &item);
    void swap(uint16_t swap_position);
    bool peek(data_t &item) const;
    uint16_t get_count() const;
    bool is_empty() const;
    bool is_full() const;
};

template <typename data_t, uint16_t size>
bool FSQueue<data_t, size>::push(const data_t &item, const uint16_t position)
{
  bool result = false;

  if (!is_full())
  {
    data[tail] = item;
    incr_queue_param(tail);
    ++count;
    if (position > 0u)
    {
      swap(position);
    }
    result = true;
  }
  return result;
}

template <typename data_t, uint16_t size>
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

template <typename data_t, uint16_t size>
bool FSQueue<data_t, size>::remove()
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

template <typename data_t, uint16_t size>
void FSQueue<data_t, size>::swap(uint16_t swap_position)
{
  data_t temp = data[head];

  if (swap_position >= count)
  {
    swap_position = static_cast<uint16_t>(count - 1u);
  }
  if (!is_empty() && (swap_position > 0u))
  {
    for (uint16_t i = 0; i < swap_position; ++i)
    {
      data[get_queue_idx(static_cast<uint16_t>(i + head))] = data[get_queue_idx(static_cast<uint16_t>(i + head + 1))];
    }
    data[get_queue_idx(static_cast<uint16_t>(swap_position + head))] = temp;
  }
}

template <typename data_t, uint16_t size>
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

template <typename data_t, uint16_t size>
uint16_t FSQueue<data_t, size>::get_count() const
{
  return count;
}

template <typename data_t, uint16_t size>
bool FSQueue<data_t, size>::is_empty() const
{
  return (count == 0);
}

template <typename data_t, uint16_t size>
bool FSQueue<data_t, size>::is_full() const
{
  return (count == size);
}

#endif // QUEUE_HPP
