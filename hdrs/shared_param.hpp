/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:39:08 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/10 08:47:00 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "p_iterator.hpp"

#include <cstdint>

struct	ssv_message_t
{
	int16_t		iterator;
	uint16_t	param_num;
	int32_t		param_val;
};

struct __attribute__((packed))	ssrv_message_t
{
	int16_t		param_num;
	int32_t		param_val;
};

struct __attribute__((packed))	sse_message_t
{
	int8_t		error_code;
	uint16_t	param_num;
};

class SharedParam
{
	public:
		enum e_errorcode
		{
			OUT_OF_RANGE_SSV,
			OUT_OF_RANGE_SSRV,
			NEW_VAL_REQ_NOT_ALLOWED,
		};
		SharedParam(uint16_t p_num = 0);
		bool		get_ssv_m(ssv_message_t& message);
		bool		get_ssrv_m(ssrv_message_t& message, int32_t new_value = 0);
		bool		get_sse_m(sse_message_t& message);
		bool		handle_ssv_m(ssv_message_t& message, uint16_t idx, uint16_t idx_can);
		bool		handle_ssrv_m(ssrv_message_t& message);
		bool		handle_sse_m(sse_message_t& message);
		bool		accept_new_value(int32_t new_value);
	private:
		uint8_t			err_code;
		uint16_t		param_num;
		P_Iterator	iterator;
		bool		is_req_update_param_value(ssv_message_t& message, uint16_t idx, uint16_t idx_can);
		int32_t	get_param_value();
		int32_t	get_param_max_value();
		void		set_param_value(int32_t p_value);
};