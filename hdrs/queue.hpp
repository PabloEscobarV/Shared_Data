/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 07:56:22 by BlackRider        #+#    #+#             */
/*   Updated: 2025/07/17 23:19:35 by Pablo Escob      ###   ########.fr       */
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
		tail = (tail + 1) % size;
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
		head = (head + 1) % size;
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
		head = (head + 1) % size;
		--count;
		result = true;
	}
	return result;
}

template <typename data_t, uint8_t size>
void FSQueue<data_t, size>::swap(uint16_t swap_item)
{
	if (count <= 1 || swap_item == 0) 
		return; // Nothing to swap
	
	// Ensure swap_item is within valid range  
	if (swap_item >= count)
	{
		swap_item = count - 1;
	}
	
	// Store the head element that will be moved to swap position
	data_t head_element = data[head];
	
	// Shift all elements from position 1 to swap_item one position left
	// This moves elements [1,2,3] to positions [0,1,2] when swap_item=3
	for (uint16_t i = 0; i < swap_item; i++)
	{
		uint8_t current_idx = (head + i) % size;
		uint8_t next_idx = (head + i + 1) % size;
		data[current_idx] = data[next_idx];
	}
	
	// Place the original head element at the swap position
	uint8_t final_swap_idx = (head + swap_item) % size;
	data[final_swap_idx] = head_element;
	
	// Result: [0,1,2,3,4,5,6] with swap_item=3 becomes [1,2,3,0,4,5,6]
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