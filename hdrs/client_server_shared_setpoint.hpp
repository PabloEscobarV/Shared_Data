/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client_server_shared_setpoint.hpp                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 07:19:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/11 10:29:25 by blackrider       ###   ########.fr       */
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
        // The map will store p_num as key and p_val as value.
        std::unordered_map<uint16_t, uint32_t> p_data;
        static constexpr	uint32_t	MAX_VALUE = 99999;

    public:
        // The map is default-constructed, so an explicit constructor is not needed
        // unless you have other initialization to do.
        // ParamData() = default;

        void		set_param_value(uint16_t p_num, uint32_t p_val);
        uint32_t	get_param_value(uint16_t p_num) const;
        bool		is_param_max_value_ok(uint16_t p_num, uint32_t setpoint_v) const;
};

template <uint16_t count>
void	ParamData<count>::set_param_value(uint16_t p_num, uint32_t p_val)
{
    // This will insert a new element if p_num doesn't exist,
    // or update the existing one.
    p_data[p_num] = p_val;
}

template <uint16_t count>
uint32_t	ParamData<count>::get_param_value(uint16_t p_num) const
{
    // Use find() to avoid inserting a new element if the key is not found.
    auto it = p_data.find(p_num);
    if (it != p_data.end())
    {
        return it->second; // Return the found value
    }
    return 0; // Return a default value if not found
}

template <uint16_t count>
bool	ParamData<count>::is_param_max_value_ok(uint16_t /*p_num*/, uint32_t setpoint_v) const
{
    return (setpoint_v <= MAX_VALUE);
}

ParamData<P_COUNT>	*param_data;

#endif // CLIENT_SERVER_SHARED_SETPOINT_HPP