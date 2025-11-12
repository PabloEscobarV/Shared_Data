/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   param_data.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 07:19:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/11/07 15:01:07 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_SERVER_SHARED_SETPOINT_HPP
#define CLIENT_SERVER_SHARED_SETPOINT_HPP

#include "../hdrs/test.hpp" // Include the test header for P_COUNT definition

#include <cstdint>
#include <unordered_map>

class	ParamData
{
    struct param_t
    {
        uint16_t	p_num;
        uint32_t	p_val;
				uint32_t 	max_val;
				uint32_t	min_val;
    };
    std::unordered_map<uint16_t, param_t> p_data;
    int32_t	MAX_VALUE = 99999;
	public:
		uint32_t	get_param_value(uint16_t idx) const;
		uint16_t	get_param_num(uint16_t idx) const;
		uint16_t	get_param_idx(uint16_t p_num) const;
		uint32_t	get_param_max_value(const uint16_t idx) const;
		uint32_t	get_param_min_value(const uint16_t idx) const;
		void			set_param_max_value(uint32_t max_val, const uint16_t idx);
		void			set_param_min_value(uint32_t min_val, const uint16_t idx);
		void			set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val);
		void			set_param_data(uint16_t idx,
															uint16_t p_num,
															uint32_t p_val,
															uint32_t max_val,
															uint32_t min_val);
		bool			is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const;
		static inline uint32_t invalid_param_value() { return UINT32_MAX; }
};

extern	ParamData	*param_data;

#endif // CLIENT_SERVER_SHARED_SETPOINT_HPP