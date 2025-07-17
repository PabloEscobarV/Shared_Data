/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:45:02 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/17 23:19:35 by Pablo Escob      ###   ########.fr       */
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

SharedParam::SharedParam(uint16_t p_num) : param_num(p_num), err_code(0), ssrv_counter(0), new_param_value(0)
{
	
}

void	SharedParam::init(uint16_t p_num)
{
	param_num = p_num;
	err_code = 0;
	ssrv_counter = 0;
	new_param_value = 0;
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
	bool	result = get_param_max_value() >= message.param_val;  // Check if incoming value is within our range

	if (result)
	{
		if (is_req_update_param_value(message, idx, idx_can))
		{
			// Accept the parameter update
			set_param_value(message.param_val);
			
			#ifdef DEBUG_SSV  
			mtx_out.lock();
			cout << "------========++++ SSV ACCEPTED ++++========------" << endl;
			cout << "PID: " << getpid() << endl
						<< "PID CAN: " << idx_can << endl
						<< "PARAM NUMBER: " << message.param_num << endl
						<< "NEW PARAM VALUE: " << message.param_val << endl
						<< "ITERATOR: " << message.iterator << endl;
			cout << "------========++++ SSV ACCEPTED ++++========------" << endl;
			mtx_out.unlock();
			#endif
		}
	}
	else
	{
		// Incoming value exceeds our max - set error for SSE message
		set_bit(err_code, OUT_OF_RANGE_SSV, true);
	}
	return result;
}

bool	SharedParam::handle_ssrv_m(const ssrv_message_t& message)
{
	bool	result = get_param_max_value() >= message.param_val;

	if (result)
	{
		// SSRV request is valid - will be processed after 500ms wait
		#ifdef DEBUG_SSRV
		mtx_out.lock();
		cout << "------========++++ SSRV ACCEPTED ++++========------" << endl;
		cout << "PID: " << getpid() << " - SSRV param #" << message.param_num 
		     << " value " << message.param_val << " accepted" << endl;
		cout << "------========++++ SSRV ACCEPTED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	else
	{
		// SSRV value exceeds our maximum - set error flag for SSE response
		set_bit(err_code, OUT_OF_RANGE_SSRV, true);
		
		#ifdef DEBUG_SSRV
		mtx_out.lock();
		cout << "------========++++ SSRV REJECTED ++++========------" << endl;
		cout << "PID: " << getpid() << " - SSRV param #" << message.param_num 
		     << " value " << message.param_val << " exceeds max " << get_param_max_value() << endl;
		cout << "------========++++ SSRV REJECTED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	return result;
}

bool	SharedParam::handle_sse_m(const sse_message_t& message)
{
	// Handle SSV out of range errors
	if (get_bit(message.error_code, OUT_OF_RANGE_SSV))
	{
		// Another process rejected our SSV value - log for debugging
		#ifdef DEBUG_SSE
		mtx_out.lock();
		cout << "------========++++ SSE: SSV REJECTED ++++========------" << endl;
		cout << "PID: " << getpid() << " - SSV value was rejected by param #" << message.param_num << endl;
		cout << "------========++++ SSE: SSV REJECTED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	
	// Handle SSRV out of range errors  
	if (get_bit(message.error_code, OUT_OF_RANGE_SSRV))
	{
		// Another process rejected our SSRV request - block the new value
		set_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED, true);
		
		#ifdef DEBUG_SSE
		mtx_out.lock();
		cout << "------========++++ SSE: SSRV REJECTED ++++========------" << endl;
		cout << "PID: " << getpid() << " - SSRV request rejected for param #" << message.param_num << endl;
		cout << "------========++++ SSE: SSRV REJECTED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	return true;
}

bool	SharedParam::accept_new_value()
{
	bool	result = !get_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED);
	
	if (result)
	{
		int32_t old_value = get_param_value();
		set_param_value(new_param_value);
		update_iterator(); // Iterator += 3 after successful SSRV
		
		#ifdef DEBUG_SSRV
		mtx_out.lock();
		cout << "------========++++ PARAM VALUE APPLIED ++++========------" << endl;
		cout << "PID: " << getpid() << " - Param #" << param_num 
		     << " changed from " << old_value << " to " << new_param_value << endl;
		cout << "New iterator: " << static_cast<int>(iterator.get_iterator_num()) << endl;
		cout << "------========++++ PARAM VALUE APPLIED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	else
	{
		#ifdef DEBUG_SSRV
		mtx_out.lock();
		cout << "------========++++ PARAM UPDATE BLOCKED ++++========------" << endl;
		cout << "PID: " << getpid() << " - Param #" << param_num 
		     << " update blocked due to SSE error" << endl;
		cout << "------========++++ PARAM UPDATE BLOCKED ++++========------" << endl;
		mtx_out.unlock();
		#endif
	}
	
	reset_ssrv_end_counter();
	return result;
}

bool	SharedParam::add_new_param_value(int32_t new_param_val, uint8_t ssrv_atmp_counter)
{
	bool	bit_check = !get_bit(err_code, SET_SSRV_COUNTER);
	
	#ifdef DEBUG_SSRV
	mtx_out.lock();
	cout << "------========++++ ADD NEW PARAM VALUE ++++========------" << endl;
	cout << "PID: " << getpid() << " - Checking if SSRV can be added" << endl;
	cout << "err_code: " << static_cast<int>(err_code) << endl;
	cout << "SET_SSRV_COUNTER bit: " << static_cast<int>(get_bit(err_code, SET_SSRV_COUNTER)) << endl;
	cout << "bit_check result: " << bit_check << endl;
	cout << "new_param_val: " << new_param_val << endl;
	cout << "ssrv_atmp_counter: " << static_cast<int>(ssrv_atmp_counter) << endl;
	cout << "------========++++ ADD NEW PARAM VALUE END ++++========------" << endl;
	mtx_out.unlock();
	#endif
	
	if (bit_check)
	{
		new_param_value = new_param_val;
		set_ssrv_end_counter(ssrv_atmp_counter);
	}
	return bit_check;
}

bool	SharedParam::is_req_update_param_value(const ssv_message_t& message, uint16_t idx, uint16_t idx_can)
{
	bool	is_req = message.param_val != get_param_value();
	
	// Debug output (can be removed for production)
	#ifdef DEBUG_SSV
	mtx_out.lock();
	cout << "------========++++ SSV REQUEST CHECK ++++========------" << endl;
	cout << "PID: " << getpid() << endl
				<< "PID CAN: " << idx_can << endl  
				<< "PARAM NUMBER: " << message.param_num << endl
				<< "PARAM VALUE: " << message.param_val << " (current: " << get_param_value() << ")" << endl
				<< "CURRENT ITERATOR: " << static_cast<int>(iterator.get_iterator_num()) << endl
				<< "CAN ITERATOR: " << static_cast<int>(message.iterator & 0xFF) << endl
				<< "ITERATOR DIFF: " << static_cast<int>(P_Iterator::get_diff(static_cast<int8_t>(message.iterator), static_cast<int8_t>(iterator))) << endl;
	cout << "------========++++ SSV REQUEST CHECK ++++========------" << endl;
	mtx_out.unlock();
	#endif

	// If different param values, check iterator and ID rules
	if (is_req)
	{
		// Rule 1: If CAN iterator > my iterator by more than 1, accept and update
		if (P_Iterator::check_iterators(message.iterator, iterator))
		{
			// CAN iterator is significantly ahead - accept update
			iterator.update_iterator(message.iterator);
			is_req = true;
		}
		// Rule 2: If same iterator or diff <=1, check process ID priority
		else if (idx_can < idx)
		{
			// Lower process ID wins - accept update
			is_req = true;
		}
		else
		{
			// Higher process ID or same ID - reject update
			is_req = false;
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
	uint16_t idx = param_data->get_param_idx(param_num);
	return param_data->get_param_max_value(idx);
}

void	SharedParam::set_param_value(int32_t p_value)
{
	param_data->set_param_value(param_data->get_param_idx(param_num), param_num, p_value);
}

uint16_t	SharedParam::get_param_num() const
{
	return param_num;
}
