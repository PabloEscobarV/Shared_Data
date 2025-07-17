/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_server_shared_setpoint.hpp                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 07:19:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/17 23:19:35 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_SERVER_SHARED_SETPOINT_HPP
#define CLIENT_SERVER_SHARED_SETPOINT_HPP

#include "test.hpp" // Include the test header for P_COUNT definition

#include <cstdint>
#include <unordered_map>
#include <mutex>

static std::mutex mtx_param_value;

// The 'count' template parameter is no longer strictly necessary for the map,
// but we'll keep it if other parts of your code rely on it.
template <uint16_t count>
class	ParamData
{
    struct param_t
    {
        uint16_t	p_num;
        uint32_t	p_val;
        uint32_t	max_val;  // Individual max value per parameter
    };
    std::unordered_map<uint16_t, param_t> p_data;
    int32_t	DEFAULT_MAX_VALUE = 99999;
	public:
		void		set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val);
		void		set_param_value_with_max(uint16_t idx, uint16_t p_num, uint32_t p_val, uint32_t max_val);
		uint32_t	get_param_value(uint16_t idx) const;
		uint16_t	get_param_num(uint16_t idx) const;
		uint16_t	get_param_idx(uint16_t p_num) const;
		uint32_t	get_param_max_value() const;
		uint32_t	get_param_max_value(uint16_t idx) const;  // Get max value for specific parameter
		void		set_param_max_value(uint32_t max_val);
		bool		is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const;
};

template <uint16_t count>
void	ParamData<count>::set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val)
{
	mtx_param_value.lock();
	auto it = p_data.find(idx);
	if (it != p_data.end()) {
		// Keep existing max_val if updating existing parameter
		p_data[idx] = { p_num, p_val, it->second.max_val };
	} else {
		// Use default max value for new parameter
		p_data[idx] = { p_num, p_val, static_cast<uint32_t>(DEFAULT_MAX_VALUE) };
	}
	mtx_param_value.unlock();
}

template <uint16_t count>
void	ParamData<count>::set_param_value_with_max(uint16_t idx, uint16_t p_num, uint32_t p_val, uint32_t max_val)
{
	mtx_param_value.lock();
	p_data[idx] = { p_num, p_val, max_val };
	mtx_param_value.unlock();
}

template <uint16_t count>
uint32_t	ParamData<count>::get_param_value(uint16_t idx) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end())
	{
		return it->second.p_val; // Return the found value
	}
	return 0; // Return a default value if not found
}

template <uint16_t count>
uint16_t	ParamData<count>::get_param_num(uint16_t idx) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end()) {
		return it->second.p_num;
	}
	return 0; // Return default value if not found
}

template <uint16_t count>
uint32_t	ParamData<count>::get_param_max_value() const
{
	return DEFAULT_MAX_VALUE;
}

template <uint16_t count>
uint32_t	ParamData<count>::get_param_max_value(uint16_t idx) const
{
	mtx_param_value.lock();
	auto it = p_data.find(idx);
	uint32_t result = (it != p_data.end()) ? it->second.max_val : DEFAULT_MAX_VALUE;
	mtx_param_value.unlock();
	return result;
}

template <uint16_t count>
void	ParamData<count>::set_param_max_value(uint32_t max_val)
{
	DEFAULT_MAX_VALUE = max_val;
}

template <uint16_t count>
uint16_t	ParamData<count>::get_param_idx(uint16_t p_num) const
{
	for (const auto& pair : p_data)
	{
		if (pair.second.p_num == p_num)
		{	
			return pair.first; // Return the index if found
		}
	}
	return count; // Return count if not found, indicating an invalid index
}

template <uint16_t count>
bool	ParamData<count>::is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const
{
	auto it = p_data.find(idx);
	if (it != p_data.end()) {
		return (it->second.p_val <= it->second.max_val);
	}
	return false; // Return false if parameter not found
}

extern ParamData<P_COUNT>	*param_data;

#endif // CLIENT_SERVER_SHARED_SETPOINT_HPP