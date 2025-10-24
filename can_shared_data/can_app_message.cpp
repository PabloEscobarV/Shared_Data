/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_app_message.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 21:18:43 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/24 21:28:17 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "can_app_message.hpp"

#include <cstring>

Can_app_message::Can_app_message(const uint8_t* ptr_data,
																 const uint8_t data_len,
																 const Can_app_message_type msg_type,
																 const uint16_t msg_flags,
																 const uint8_t src_controller_id,
																 const uint16_t message_sid,
																 const uint8_t* msgs_size_count)
	: data_length(data_len),
		message_type(msg_type),
		flags(msg_flags),
		source_controller_id(src_controller_id),
		sid(message_sid)
{
	memset(data, 0, sizeof(data));
	memset(messages_size_count, 0, sizeof(messages_size_count));
	if (ptr_data != nullptr && data_len <= MAX_DATA_LEN)
	{
		memcpy(data, ptr_data, data_length);
	}
	if (msgs_size_count != nullptr)
	{
		memcpy(messages_size_count, msgs_size_count, sizeof(messages_size_count));
	}
}

const uint8_t* Can_app_message::get_data_pointer() const
{
	return data;
}

uint8_t Can_app_message::get_data_len() const
{
	return data_length;
}

Can_app_message::Can_app_message_type Can_app_message::get_message_type() const
{
	return message_type;
}

uint16_t Can_app_message::get_flags() const
{
	return flags;
}

uint8_t Can_app_message::get_source_controller_id() const
{
	return source_controller_id;
}

uint16_t Can_app_message::get_sid() const
{
	return sid;
}

const uint8_t* Can_app_message::get_messages_size_count() const
{
	return messages_size_count;
}
