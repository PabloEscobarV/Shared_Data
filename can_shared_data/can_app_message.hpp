/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_app_message.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 21:13:34 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/27 03:36:11 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAN_APP_MESSAGE_HPP
#define CAN_APP_MESSAGE_HPP

#include <cstdint>

class Can_app_message
{
	public:
		enum class Can_app_message_type : uint8_t
		{
			Shared_data,
			Other
		};

		static const uint8_t  MAX_DATA_LEN = 59;
		static const uint8_t  MAX_FLAGS_COUNT = 16;

		Can_app_message(const uint8_t* ptr_data = nullptr,
										const uint8_t data_len = 0,
										const Can_app_message_type msg_type = Can_app_message_type::Other,
										const uint16_t msg_flags = 0,
										const uint8_t src_controller_id = 0,
										const uint16_t message_sid = 0,
										const uint8_t* ptr_message_size_count = nullptr);

		const uint8_t* 				get_data_pointer() const;
		uint8_t        				get_data_len() const;
		Can_app_message_type 	get_message_type() const;
		uint16_t       				get_flags() const;
		uint8_t        				get_source_controller_id() const;
		uint16_t       				get_sid() const;
		const uint8_t* 				get_messages_size_count() const;
		inline void						set_pid(uint8_t pid) { source_controller_id = pid; }

	private:
		
		uint8_t  							data[MAX_DATA_LEN];
		uint8_t           		data_length;
		Can_app_message_type 	message_type;
		uint16_t          		flags;
		uint8_t          			source_controller_id;
		uint16_t         			sid;
		uint8_t          			messages_size_count[MAX_FLAGS_COUNT];
};

#endif // CAN_APP_MESSAGE_HPP