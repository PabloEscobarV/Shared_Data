/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_server_shared_setpoint.hpp                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 07:19:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/11 21:20:08 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_SERVER_SHARED_SETPOINT_HPP
#define CLIENT_SERVER_SHARED_SETPOINT_HPP

#include "test.hpp" // Include the test header for P_COUNT definition

#include <cstdint>
#include <unordered_map>

// The 'count' template parameter is no longer strictly necessary for the map,
// but we'll keep it if other parts of your code rely on it.
template <uint16_t count>
class	ParamData
{
    struct param_t
    {
        uint16_t	p_num;
        uint32_t	p_val;
    };
    std::unordered_map<uint16_t, param_t> p_data;
    static constexpr	uint32_t	MAX_VALUE = 99999;
	public:
		void		set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val);
		uint32_t	get_param_value(uint16_t idx) const;
		uint16_t	get_param_num(uint16_t idx) const;
		bool		is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const;
};

template <uint16_t count>
void	ParamData<count>::set_param_value(uint16_t idx, uint16_t p_num, uint32_t p_val)
{
	p_data[idx] = { p_num, p_val };
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
bool	ParamData<count>::is_param_max_value_ok(uint16_t idx, uint32_t setpoint_v) const
{
    auto it = p_data.find(idx);
    if (it != p_data.end()) {
        return (it->second.p_val <= MAX_VALUE);
    }
    return false; // Return false if parameter not found
}

ParamData<P_COUNT>	*param_data;

#endif // CLIENT_SERVER_SHARED_SETPOINT_HPP