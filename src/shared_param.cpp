/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:45:02 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/10 08:51:08 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_param.hpp"
#include "../hdrs/bit_operations.hpp"

SharedParam::SharedParam(uint16_t p_num) : param_num(p_num)
{
	
}

bool	SharedParam::get_ssv_m(ssv_message_t& message)
{
	message.iterator = iterator;
	message.param_num = param_num;
	message.param_val = get_param_value();
	++iterator;
	return true;
}

bool	SharedParam::get_ssrv_m(ssrv_message_t& message, int32_t new_value)
{
	message.param_num = param_num;
	message.param_val = new_value;
	return true;
}

bool	SharedParam::get_sse_m(sse_message_t& message)
{
	message.error_code = err_code;
	message.param_num = param_num;
	return true;
}

bool	SharedParam::handle_ssv_m(ssv_message_t& message, uint16_t idx, uint16_t idx_can)
{
	bool	result = get_param_max_value() >= get_param_value();

	if (result)
	{
		if (is_req_update_param_value(message, idx, idx_can))
		{
			set_param_value(message.param_val);
		}
	}
	else
	{
		set_bit(&err_code, OUT_OF_RANGE_SSV, true);
	}
	return result;
}

bool	SharedParam::handle_ssrv_m(ssrv_message_t& message)
{
	bool	result = get_param_max_value() >= message.param_val;

	if (!result)
	{
		set_bit(&err_code, OUT_OF_RANGE_SSRV, true);
	}
	return result;
}

bool	SharedParam::handle_sse_m(sse_message_t& message)
{
	if (get_bit(message.error_code, OUT_OF_RANGE_SSV))
	{

	}
	if (get_bit(message.error_code, OUT_OF_RANGE_SSRV))
	{
		set_bit(&err_code, NEW_VAL_REQ_NOT_ALLOWED, true);
	}
	return true;
}

bool	SharedParam::accept_new_value(int32_t new_value)
{
	bool	result = !get_bit(err_code, NEW_VAL_REQ_NOT_ALLOWED);
	
	if (result)
	{
		set_param_value(new_value);
	}
	return result;
}

bool	SharedParam::is_req_update_param_value(ssv_message_t& message, uint16_t idx, uint16_t idx_can)
{
	bool	is_req = message.param_val != get_param_value();
	
	if (!iterator.update_iterator(message.iterator))
	{
		if (iterator.check_iterator(iterator, message.iterator))
		{
			is_req = false;
		}
		if (is_req && (idx_can > idx))
		{
			is_req = false;
		}
	}
	return is_req;
}

int32_t	SharedParam::get_param_value()
{

}

int32_t	SharedParam::get_param_max_value()
{

}

void	SharedParam::set_param_value(int32_t p_value)
{

}
