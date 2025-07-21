/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 07:56:22 by BlackRider        #+#    #+#             */
/*   Updated: 2025/07/21 08:57:18 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <cstdint>

template <typename data_t, uint8_t size>
class FSQueue
{
	private:
		uint8_t	head;
		uint8_t	tail;
		uint8_t	count;
		data_t 	data[size];
		inline void			incr_queue_param(uint8_t& param) { param = (param + 1) % size; }
		inline uint8_t	get_queue_idx(uint8_t i) { return i % size; }
	public:
		FSQueue() : head(0), tail(0), count(0) {}
		bool	push(const data_t &item);
		bool	pop();
		bool	pop(data_t &item);
		void	swap(uint16_t swap_item);
		bool	peek(data_t &item) const;
		bool	peek_second(data_t &item) const;
		uint8_t get_count() const;
		bool is_empty() const;
		bool is_full() const;
};

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
			data[get_queue_idx(i + head)] = data[get_queue_idx(i + head + 1)];
		}
		data[get_queue_idx(swap_item + head)] = temp;
	}
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::peek(data_t& item) const
{
	bool	result = false;
	
	if (!is_empty())
	{
		item = data[head];
		result = true;
	}
	return result;
}

template <typename data_t, uint8_t size>
bool FSQueue<data_t, size>::peek_second(data_t& item) const
{
	bool	result = false;
	
	if (count > 1)
	{
		item = data[(head + 1) % size];
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