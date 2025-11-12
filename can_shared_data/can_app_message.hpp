/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_app_message.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 21:13:34 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/11/07 17:23:58 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAN_APP_MESSAGE_HPP
#define CAN_APP_MESSAGE_HPP

#include "../bit/bit.hpp"

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
		inline void						reset_offset() { offset = 0; }
		template <typename data_t>
		bool get_data(const uint16_t msg_type, data_t& dest);

	private:
		
		uint8_t  							data[MAX_DATA_LEN];
		uint8_t          			messages_size_count[MAX_FLAGS_COUNT];
		uint16_t          		flags;
		uint16_t         			sid;
		Can_app_message_type 	message_type;
		uint8_t          			source_controller_id;
		uint8_t           		data_length;
		uint8_t 				 			offset;
};

template <typename data_t>
bool Can_app_message::get_data(const uint16_t msg_type, data_t& dest)
{
  bool result = false;

  if (Bit::test(flags, msg_type) && (offset + sizeof(data_t) <= data_length))
  {
    memcpy(&dest, data + offset, sizeof(data_t));
    offset += sizeof(data_t);
    result = true;
  }
  return result;
}

#endif // CAN_APP_MESSAGE_HPP