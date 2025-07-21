/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:54:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/21 14:26:02 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "shared_param.hpp"
#include "queue.hpp"
#include "test.hpp"

#include <cstdint>
#include <cstring>

#include <iostream>
#include <unistd.h>

using namespace std;

using idx_t = uint8_t;

enum message_type_t
{
	SSV_MESSAGE = 1,
	SSRV_MESSAGE,
	SSE_MESSAGE,
};

struct can_data_t
{
	uint8_t		message_type;
	uint8_t		data[sizeof(ssv_message_t)];
	uint16_t	data_len;

	uint16_t	idx;
	uint16_t	idx_can;
};

template <uint16_t count>
class SharedData
{
	public:
		SharedData();
		void	set_param_num(uint16_t param_num);
		void	period_counter();
		bool	add_ssrv_message(uint16_t param_num, int32_t new_param_val);
		bool	get_messages(can_data_t &can_data);
		bool	handle_messages(can_data_t &can_data);

		// TEST PURPOSES ONLY
		void	set_iterator(uint16_t idx, int16_t i)
		{
			shared_params[idx].set_iterator(i);
		}
		uint16_t get_iterator(uint16_t idx)
		{
			return shared_params[idx].get_param_iterator(idx);
		}	
	private:
		struct ssrv_service_t
		{
			idx_t	idx;
		};
		struct	sse_service_t
		{
			uint8_t	counter;
			uint8_t	idx;
		};
		static const uint8_t	QUEUE_SIZE = count / 5 + 1;
		static const uint8_t	SSV_PERIOD = 5;	
		static const uint8_t	SSRV_PERIOD = 2;
		static const uint8_t	SSE_PERIOD = 5;
		static const uint8_t	SSRV_ATTEMPTS = 3;
		static const uint8_t	SSRV_WAIT_TICKS = 25;
		uint8_t		tick;
		uint16_t	idx_ssv;
		SharedParam	shared_params[count];
		FSQueue<ssrv_service_t, QUEUE_SIZE>	ssrv_queue;
		FSQueue<sse_service_t, QUEUE_SIZE>	sse_queue;
		bool			get_ssv_message(ssv_message_t &message);
		bool			get_ssrv_message(ssrv_message_t &message);
		bool			get_sse_message(sse_message_t& message);
		bool			set_ssv_message(can_data_t &can_data);
		bool			set_ssrv_message(can_data_t &can_data);
		bool			set_sse_message(can_data_t &can_data);
		bool			handle_ssv_message(const ssv_message_t &message, const uint16_t id, const uint16_t id_can);
		bool			handle_ssrv_message(const ssrv_message_t &message);
		bool			handle_sse_message(const sse_message_t& message);
		idx_t			get_idx(uint16_t p_num) const;
		uint16_t	check_ssrv_end_counters();
};

template <uint16_t count>
SharedData<count>::SharedData() : tick(0), idx_ssv(0)
{

}

template <uint16_t count>
void	SharedData<count>::set_param_num(uint16_t param_num)
{
	shared_params[idx_ssv].init(param_num);
	idx_ssv = (idx_ssv + 1) % count;
}

template <uint16_t count>
void	SharedData<count>::period_counter()
{
	tick++;
}

template <uint16_t count>
bool	SharedData<count>::add_ssrv_message(uint16_t param_num, int32_t new_param_val)
{
	ssrv_service_t	new_message
	{
		.idx = get_idx(param_num),
	};
	bool	result = false;

	if (new_message.idx < count)
	{
		shared_params[new_message.idx].add_new_param_value(new_param_val, SSRV_ATTEMPTS);
		result = ssrv_queue.push(new_message);
		cout << "PID: " << get_pid() 
					<< " PARAM NUMBER: " << param_num
					<< " NEW PARAM VALUE: " << new_param_val
					<< endl;
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::get_messages(can_data_t &can_data)
{
	bool	result = false;

	result = set_sse_message(can_data);
	if (!result)
	{
		result = set_ssrv_message(can_data);
	}
	if (!result)
	{
		result = set_ssv_message(can_data);
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_messages(can_data_t &can_data)
{
	ssv_message_t		ssv_message;
	ssrv_message_t	ssrv_message;
	sse_message_t		sse_message;
	bool	result = false;

	if (can_data.message_type == SSV_MESSAGE)
	{
		memcpy(&ssv_message, can_data.data, can_data.data_len);
		// mtx_out.lock();
		// cout << "------========++++ SSV RECEIVE ++++========------" << endl;
		// cout << "PID: " << get_pid() << endl
		// 			<< "PID CAN: " << can_data.idx_can << endl
		// 			<< "PARAM NUMBER: " << ssv_message.param_num << endl
		// 			<< "PARAM VALUE: " << ssv_message.param_val << endl
		// 			<< "ITERATOR: " << ssv_message.iterator << endl
		// 			<< endl;
		// cout << "------========++++ SSV RECEIVE ++++========------" << endl;
		// mtx_out.unlock();
		handle_ssv_message(ssv_message, can_data.idx, can_data.idx_can);
	}
	if (can_data.message_type == SSRV_MESSAGE)
	{
		memcpy(&ssrv_message, can_data.data, can_data.data_len);
		mtx_out.lock();
		cout << "------========++++ SSRV RECEIVE ++++========------" << endl;
		cout << "PID: " << get_pid() << endl
					<< " PARAM NUMBER: " << ssrv_message.param_num << endl
					<< " PARAM VALUE: " << ssrv_message.param_val << endl
					<< endl;
		cout << "------========++++ SSRV RECEIVE ++++========------" << endl;
		mtx_out.unlock();
		handle_ssrv_message(ssrv_message);
	}
	if (can_data.message_type == SSE_MESSAGE)
	{
		memcpy(&sse_message, can_data.data, can_data.data_len);
		handle_sse_message(sse_message);
	}
	return true;
}

template <uint16_t count>
bool	SharedData<count>::get_ssv_message(ssv_message_t &message)
{
	bool	result = false;
	uint16_t	idx = check_ssrv_end_counters();

	if (idx < count)
	{
		shared_params[idx].accept_new_value();
	}
	else
	{
		idx = idx_ssv;
		idx_ssv = (idx_ssv + 1) % count;
	}
	result = shared_params[idx].get_ssv_m(message);
	// mtx_out.lock();
	// cout << "PID: " << get_pid() 
	// 			<< "PARAM NUMBER: " << message.param_num
	// 			<< " PARAM VALUE: " << message.param_val
	// 			<< " ITERATOR: " << message.iterator
	// 			<< endl;
	// mtx_out.unlock();
	return result;
}

template <uint16_t count>
bool	SharedData<count>::get_ssrv_message(ssrv_message_t &message)
{
	uint8_t	queue_counter = 0;
	ssrv_service_t	ssrv_service;
	bool	result = ssrv_queue.pop(ssrv_service);

	if (result)
	{
		shared_params[ssrv_service.idx].get_ssrv_end_counter(queue_counter);
		shared_params[ssrv_service.idx].get_ssrv_m(message);
		if ((--queue_counter) > 0)
		{
			ssrv_queue.push(ssrv_service);
			ssrv_queue.swap(SSRV_PERIOD);
			shared_params[ssrv_service.idx].set_ssrv_end_counter(queue_counter);
		}
		else
		{
			shared_params[ssrv_service.idx].set_ssrv_end_counter(SSRV_WAIT_TICKS);
		}
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::get_sse_message(sse_message_t &message)
{
	sse_service_t	sse_service;
	bool	result = sse_queue.pop(sse_service);

	if (result)
	{
		shared_params[sse_service.idx].get_sse_m(message);
		sse_service.counter--;
		if (sse_service.counter > 0)
		{
			sse_queue.push(sse_service);
			sse_queue.swap(SSE_PERIOD);
		}
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::set_ssv_message(can_data_t &can_data)
{
	ssv_message_t	message;
	bool	result = false;

	if ((tick % SSV_PERIOD == 0) && get_ssv_message(message))
	{
		memcpy(&can_data.data, &message, sizeof(ssv_message_t));
		can_data.data_len = sizeof(ssv_message_t);
		can_data.message_type = SSV_MESSAGE;
		result = true;
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::set_ssrv_message(can_data_t &can_data)
{
	ssrv_message_t	message;
	bool	result = false;

	if ((tick % SSRV_PERIOD == 0) && get_ssrv_message(message))
	{
		memcpy(&can_data.data, &message, sizeof(ssrv_message_t));
		can_data.data_len = sizeof(ssrv_message_t);
		can_data.message_type = SSRV_MESSAGE;
		result = true;
		mtx_out.lock();
		cout << "------========++++ SSRV SEND ++++========------" << endl;
		cout << "PID: " << get_pid() << endl
					<< " PARAM NUMBER: " << message.param_num << endl
					<< " PARAM VALUE: " << message.param_val << endl
					<< endl;
		cout << "------========++++ SSRV SEND ++++========------" << endl;
		mtx_out.unlock();
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::set_sse_message(can_data_t &can_data)
{
	sse_message_t	message;
	bool	result = false;

	if (get_sse_message(message))
	{
		memcpy(&can_data.data, &message, sizeof(sse_message_t));
		can_data.data_len = sizeof(sse_message_t);
		can_data.message_type = SSE_MESSAGE;
		result = true;
		mtx_out.lock();
		cout << "------========++++ SSE SEND ++++========------" << endl;
		cout << "PID: " << get_pid() << endl
					<< " PARAM NUMBER: " << message.param_num << endl
					<< " ERROR CODE: " << static_cast<int>(message.error_code) << endl
					<< endl;
		cout << "------========++++ SSE SEND ++++========------" << endl;
		mtx_out.unlock();
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_ssv_message(const ssv_message_t &message,
																						const uint16_t id,
																						const uint16_t id_can)
{
	sse_service_t	sse_service
	{
		.counter = SSRV_ATTEMPTS,
		.idx = get_idx(message.param_num),
	};
	bool	result = false; 
	
	if (sse_service.idx < count)
	{
		result = shared_params[sse_service.idx].handle_ssv_m(message, id, id_can);
		if (!result)
		{
			sse_queue.push(sse_service);
		}
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_ssrv_message(const ssrv_message_t &message)
{
	sse_service_t	sse_service
	{
		.counter = 1,
		.idx = get_idx(message.param_num),
	};
	bool	result;
	
	if (sse_service.idx < count)
	{
		result = shared_params[sse_service.idx].handle_ssrv_m(message);
		if (!result)
		{
			sse_queue.push(sse_service);
		}
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_sse_message(const sse_message_t &message)
{
	bool	result = false;
	uint16_t idx = get_idx(message.param_num);
	
	if (idx < count)
	{
		result = shared_params[idx].handle_sse_m(message);
	}
	return result;
}

template <uint16_t count>
idx_t	SharedData<count>::get_idx(uint16_t p_num) const
{
	idx_t	left = 0;
	idx_t	right = count;
	idx_t 	mid = 0;
	
	while (left < right)
	{
		mid = left + (right - left) / 2;
		if (shared_params[mid].get_param_num() == p_num)
		{
			break ;
		}
		if (shared_params[mid].get_param_num() < p_num)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
	}
	if (shared_params[mid].get_param_num() != p_num)
	{
		mid = count;
	}
	return mid;
}

template <uint16_t count>
uint16_t	SharedData<count>::check_ssrv_end_counters()
{
	uint8_t		ssrv_counter = 0;
	uint16_t	idx = 0;
	
	while (idx < count)
	{
		if (shared_params[idx].get_ssrv_end_counter(ssrv_counter))
		{
			if (P_Iterator::get_diff(static_cast<int8_t>(tick), static_cast<int8_t>(ssrv_counter))
					>= SSRV_WAIT_TICKS)
			{
				break ;
			}
		}
		++idx;
	}
	return idx;
}

// extern SharedData<P_COUNT> *shared_data;

#endif