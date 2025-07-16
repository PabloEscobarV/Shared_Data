/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:45:02 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/16 11:01:28 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_param.hpp"
#include "../hdrs/bit_operations.hpp"

#include "../hdrs/client_server_shared_setpoint.hpp"

#include <iostream>
#include <mutex>
#include <unistd.h>

using std::cout;
using std::endl;

SharedParam::SharedParam(uint16_t p_num) : param_num(p_num)
{
	
}

void	SharedParam::init(uint16_t p_num)
{
	param_num = p_num;
}

bool	SharedParam::get_ssv_m(ssv_message_t& message)
{
	message.iterator = iterator;
	message.param_num = param_num;
	message.param_val = get_param_value();
	++iterator;
	return true;
}

bool	SharedParam::get_ssrv_m(ssrv_message_t& message)
{
	message.param_num = param_num;
	message.param_val = new_param_value;
	return true;
}

bool	SharedParam::get_sse_m(sse_message_t& message)
{
	message.error_code = err_code;
	message.param_num = param_num;
	return true;
}

bool	SharedParam::handle_ssv_m(const ssv_message_t& message, uint16_t idx, uint16_t idx_can)
{
	bool	result = get_param_max_value() >= get_param_value();

	if (result)
	{
		if (is_req_update_param_value(message, idx, idx_can))
		{
			mtx_out.lock();
			cout << "------========++++ SSV RECEIVE ++++========------" << endl;
			cout << "PID: " << getpid() << endl
						<< "PID CAN: " << idx_can << endl
						<< "PARAM NUMBER: " << message.param_num << endl
						<< "PARAM VALUE: " << message.param_val << endl
						<< "ITERATOR: " << message.iterator << endl
						<< endl;
			cout << "------========++++ SSV RECEIVE ++++========------" << endl;
			mtx_out.unlock();
			set_param_value(message.param_val);
		}
	}
	else
	{
		set_bit(err_code, OUT_OF_RANGE_SSV, true);
	}
	return result;
}

bool	SharedParam::handle_ssrv_m(const ssrv_message_t& message)
{
	bool	result = get_param_max_value() >= message.param_val;

	if (!result)
	{
		set_bit(err_code, OUT_OF_RANGE_SSRV, true);
	}
	return result;
}

bool	SharedParam::handle_sse_m(const sse_message_t& message)
{
	if (get_bit(message.error_code, OUT_OF_RANGE_SSV))
	{

	}
	if (get_bit(message.error_code, OUT_OF_RANGE_SSRV))
	{
		set_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED, true);
	}
	return true;
}

bool	SharedParam::accept_new_value()
{
	bool	result = !get_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED);
	
	if (result)
	{
		set_param_value(new_param_value);
		update_iterator();
	}
	reset_ssrv_end_counter();
	return result;
}

bool	SharedParam::add_new_param_value(int32_t new_param_val, uint8_t ssrv_atmp_counter)
{
	bool	result = !get_bit(err_code, SET_SSRV_COUNTER);
	
	if (result)
	{
		new_param_value = new_param_val;
		set_ssrv_end_counter(ssrv_atmp_counter);
	}
	return result;
}

bool	SharedParam::is_req_update_param_value(const ssv_message_t& message, uint16_t idx, uint16_t idx_can)
{
	bool	is_req = message.param_val != get_param_value();

	mtx_out.lock();
	cout << "PID: " << getpid() 
				<< " PID CAN: " << idx_can
				<< " PARAM NUMBER: " << message.param_num
				<< " PARAM VALUE: " << message.param_val
				<< " ITERATOR: " << message.iterator
				<< endl;
	mtx_out.unlock();
	if (!iterator.update_iterator(message.iterator))
	{
		if (P_Iterator::check_iterators(iterator, message.iterator))
		{
			is_req = false;
		}
		if (is_req && (idx_can > idx))
		{
			is_req = false;
			mtx_out.lock();
			cout << "PID: " << getpid() 
						<< " PARAM NUMBER: " << message.param_num
						<< " PARAM VALUE: " << message.param_val
						<< " ITERATOR: " << message.iterator
						<< " NOT ALLOWED: idx_can > idx" << endl;
			mtx_out.unlock();
		}
	}
	return is_req;
}

void	SharedParam::set_ssrv_end_counter(uint8_t counter)
{
	ssrv_counter = counter;
	set_bit(err_code, SET_SSRV_COUNTER, true);
}

void	SharedParam::reset_ssrv_end_counter()
{
	set_bit(err_code, SET_SSRV_COUNTER, false);
}

bool	SharedParam::get_ssrv_end_counter(uint8_t& counter) const
{
	counter = ssrv_counter;
	return get_bit(err_code, SET_SSRV_COUNTER);
}

bool SharedParam::is_new_value_allowed() const
{
	return !get_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED);
}

void SharedParam::update_iterator()
{
	iterator += SSRV_INCR_VALUE;
}

int32_t	SharedParam::get_param_value()
{
	return param_data->get_param_value(param_data->get_param_idx(param_num));
}

int32_t	SharedParam::get_param_max_value()
{
	return param_data->get_param_max_value();
}

void	SharedParam::set_param_value(int32_t p_value)
{
	param_data->set_param_value(param_data->get_param_idx(param_num), param_num, p_value);
}

uint16_t	SharedParam::get_param_num() const
{
	return param_num;
}
