/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   param_data.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 07:19:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/10/24 21:28:35 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_SERVER_SHARED_SETPOINT_HPP
#define CLIENT_SERVER_SHARED_SETPOINT_HPP

#include "test.hpp" // Include the test header for P_COUNT definition

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

void	ParamData::set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val)
{
	p_data[idx] = { p_num, p_val };
}

uint32_t	ParamData::get_param_value(uint16_t idx) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end())
	{
		return it->second.p_val; // Return the found value
	}
	return UINT32_MAX; // Return a default value if not found
}

uint16_t	ParamData::get_param_num(uint16_t idx) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end()) {
		return it->second.p_num;
	}
	return UINT16_MAX; // Return default value if not found
}

uint32_t	ParamData::get_param_max_value(const uint16_t idx) const
{
	auto it = p_data.find(idx);
	
	if (it != p_data.end()) {
		return it->second.max_val;
	}
	return UINT32_MAX; // Return default value if not found
}

uint32_t	ParamData::get_param_min_value(const uint16_t idx) const
{
	auto it = p_data.find(idx);
	
	if (it != p_data.end()) {
		return it->second.min_val;
	}
	return UINT32_MAX; // Return default value if not found
}

void	ParamData::set_param_max_value(uint32_t max_val, const uint16_t idx)
{
	auto it = p_data.find(idx);
	
	if (it != p_data.end()) {
		it->second.max_val = max_val;
	}
}

void ParamData::set_param_min_value(uint32_t min_val, const uint16_t idx)
{
	auto it = p_data.find(idx);
	
	if (it != p_data.end()) {
		it->second.min_val = min_val;
	}
}

uint16_t	ParamData::get_param_idx(uint16_t p_num) const
{
	for (const auto& pair : p_data)
	{
		if (pair.second.p_num == p_num)
		{	
			return pair.first; // Return the index if found
		}
	}
	return UINT16_MAX; // Return UINT16_MAX if not found, indicating an invalid index
}

void ParamData::set_param_data(uint16_t idx,
														uint16_t p_num,
														uint32_t p_val,
														uint32_t max_val,
														uint32_t min_val)
{
	p_data[idx] = { p_num, p_val, max_val, min_val };
}

bool	ParamData::is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end()) {
		return (it->second.p_val <= MAX_VALUE);
	}
	return false; // Return false if parameter not found
}

extern	ParamData	param_data;

#endif // CLIENT_SERVER_SHARED_SETPOINT_HPP