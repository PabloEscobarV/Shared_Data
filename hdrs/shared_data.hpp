/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:54:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/13 14:31:16 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shared_param.hpp"
#include "queue.hpp"
#include "test.hpp"

#include <cstdint>
#include <cstring>

enum message_type_t
{
	SSV_MESSAGE = 1,
	SSRV_MESSAGE,
	SSE_MESSAGE,
};

struct can_data_t
{
	uint8_t		message_type;
	uint8_t		*data;
	uint16_t	data_len;

	uint16_t	idx;
	uint16_t	idx_can;
};

template <uint16_t count>
class SharedData
{
	public:
		SharedData();
		void	period_counter();
		bool	add_ssrv_message(uint16_t param_num, int32_t new_param_val);
		bool	get_messages(can_data_t &can_data);
		bool	handle_messages(can_data_t &can_data);
	private:
		struct ssrv_service_t
		{
			uint8_t	idx;
		};
		struct	sse_service_t
		{
			uint8_t	counter;
			uint8_t	idx;
		};
		static const uint8_t	QUEUE_SIZE = count / 5;
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
		uint16_t	get_idx(uint16_t p_num) const;
		uint16_t	check_ssrv_end_counters();
};

template <uint16_t count>
SharedData<count>::SharedData() : tick(0), idx_ssv(0)
{

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
	shared_params[new_message.idx].set_ssrv_end_counter(SSRV_ATTEMPTS);
	return ssrv_queue.push(new_message);
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
		handle_ssv_message(ssv_message, can_data.idx, can_data.idx_can);
	}
	if (can_data.message_type == SSRV_MESSAGE)
	{
		memcpy(&ssrv_message, can_data.data, can_data.data_len);
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
	result = shared_params[idx].get_ssv(message);
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
	bool	result = shared_params[sse_service.idx].handle_ssv_m(message, id, id_can);

	if (!result)
	{
		sse_queue.push(sse_service);
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_ssrv_message(const ssrv_message_t &message)
{
	sse_service_t	sse_service
	{
		.counter = SSRV_ATTEMPTS,
		.idx = get_idx(message.param_num),
	};
	bool	result = shared_params[sse_service.idx].handle_ssrv_m(message);
	
	if (!result)
	{
		sse_queue.push(sse_service);
	}
	return result;
}

template <uint16_t count>
bool	SharedData<count>::handle_sse_message(const sse_message_t &message)
{
	return shared_params[get_idx(message.param_num)].handle_sse_m(message);
}

template <uint16_t count>
uint16_t	SharedData<count>::get_idx(uint16_t p_num) const
{
	uint16_t	left = 0;
	uint16_t	right = count;
	uint16_t 	mid = 0;
	
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
			if (get_diff(static_cast<int8_t>(tick), static_cast<int8_t>(ssrv_counter)) >= SSRV_WAIT_TICKS)
			{
				break ;
			}
		}
		++idx;
	}
	return idx;
}

SharedData<P_COUNT> *shared_data;