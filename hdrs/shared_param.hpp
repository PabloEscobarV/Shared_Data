/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:39:08 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/09 22:01:40 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "p_iterator.hpp"

#include <cstdint>

struct	ssv_message_t
{
	int16_t		iterator;
	uint16_t	param_num;
	uint32_t	param_val;
};

struct __attribute__((packed))	ssrv_message_t
{
	int16_t		param_num;
	uint32_t	param_val;
};

struct __attribute__((packed))	sse_message_t
{
	int8_t		error_code;
	uint16_t	param_num;
};

class SharedParam
{
	public:
		SharedParam(uint16_t p_num = 0);
		void	set_new_value_req(uint32_t new_value);
		bool	get_ssv_m(ssv_message_t& message);
		bool	get_ssrv_m(ssrv_message_t& message);
		bool	get_sse_m(sse_message_t& message);
		bool	handle_ssv_m(ssv_message_t& message, uint16_t idx, uint16_t idx_can);
		bool	handle_ssrv_m(ssrv_message_t& message);
		bool	handle_sse_m(sse_message_t& message);
	private:
		bool				is_new_value_req;
		uint16_t		param_num;
		uint32_t		new_param_value;
		P_Iterator	iterator;
		uint32_t	get_param_value();
		uint32_t	get_param_max_value();
};