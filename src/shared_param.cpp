/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:45:02 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/09 22:03:44 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_param.hpp"

SharedParam::SharedParam(uint16_t p_num) : param_num(p_num),
																					new_param_value(0),
																					is_new_value_req(false)
{
	
}

void	SharedParam::set_new_value_req(uint32_t new_value)
{
	is_new_value_req = true;
	new_param_value = new_value;
}

bool	SharedParam::get_ssv_m(ssv_message_t& message)
{
	message.iterator = iterator;
	message.param_num = param_num;
	message.param_val = get_param_value();
	++iterator;
}

bool	SharedParam::get_ssrv_m(ssrv_message_t& message)
{
	bool	is_message = false;
	
	if (is_new_value_req)
	{
		message.param_num = param_num;
		message.param_val = new_param_value;
		is_message = true;
	}
	return is_message;
}