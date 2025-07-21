/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:39:08 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/21 14:24:53 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_PARAM_HPP
#define SHARED_PARAM_HPP

#include "p_iterator.hpp"

#include <cstdint>

#include <mutex>

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
			NO_ERROR,
			OUT_OF_RANGE_SSV,
			OUT_OF_RANGE_SSRV,
			NEW_VAL_REQ_NOT_ALLOWED,
			SET_SSRV_COUNTER,
		};
		SharedParam(uint16_t p_num = 0);
		void			init(uint16_t p_num);
		bool			accept_new_value();
		bool			add_new_param_value(int32_t new_param_val, uint8_t ssrv_atmp_counter);
		void			set_ssrv_end_counter(uint8_t counter);
		void			reset_ssrv_end_counter();
		bool			get_ssrv_end_counter(uint8_t& counter) const;
		bool			is_new_value_allowed() const;
		bool			get_ssv_m(ssv_message_t& message);
		bool			get_ssrv_m(ssrv_message_t& message);
		bool			get_sse_m(sse_message_t& message);
		bool			handle_ssv_m(const ssv_message_t& message, uint16_t idx, uint16_t idx_can);
		bool			handle_ssrv_m(const ssrv_message_t& message);
		bool			handle_sse_m(const sse_message_t& message);
		uint16_t	get_param_num() const;

	// TEST PURPOSES ONLY

		void	set_iterator(int16_t i)
		{
			iterator.set_iterator(i);
		}
		uint16_t	get_param_iterator(uint16_t idx) const
		{
			return iterator.get_iterator();
		}
	private:
		static const uint8_t	SSRV_INCR_VALUE = 3;
		uint8_t			ssrv_counter;
		uint8_t			err_code;
		uint16_t		param_num;
		int32_t			new_param_value;
		P_Iterator	iterator;
		bool		is_req_update_param_value(const ssv_message_t& message, uint16_t idx, uint16_t idx_can);
		int32_t	get_param_value();
		int32_t	get_param_max_value();
		void		set_param_value(int32_t p_value);
		void		update_iterator();
};

#endif // SHARED_PARAM_HPP