/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_shared_data.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 19:53:50 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/24 21:28:13 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../shared_data/shared_data.hpp"
#include "can_app_message.hpp"

class Can_shared_data : public Shared_data
{
		static const uint8_t  MAX_DATA_LEN = 59;
		static const uint8_t  MAX_FLAGS_COUNT = 16;
		static const uint8_t  SSV_MSG_FLAG = 1 << SSV_MESSAGE;
    static const uint8_t  SSE_MSG_FLAG = 1 << SSE_MESSAGE;
    static const uint16_t SSRV_MSG_FLAGS[PACK_SIZE];

		struct can_data_t
    {
      uint8_t  data[MAX_DATA_LEN];
      uint8_t  messages_size[MAX_FLAGS_COUNT];
      uint8_t  data_len;
      uint16_t idx_can;
      uint16_t offset;
      uint16_t message_type_flags;

      can_data_t();
      can_data_t(const uint8_t *ptr_data,
                const uint8_t data_size,
                const uint16_t id_cu_received,
                const uint16_t message_flags);
      can_data_t(const uint8_t *ptr_data,
                const uint8_t (&msgs_size)[MAX_FLAGS_COUNT],
                const uint8_t data_size,
                const uint16_t id_cu_received,
                const uint16_t message_flags);
      template <typename data_t>
      bool add_data(const data_t& data_obj, const uint16_t msg_type, const uint16_t msg_flag);
      template <typename data_t>
      data_t get_data();
    };

		uint32_t	shared_data_event;
		uint16_t	can_cu_id;

		bool get_messages(can_data_t &can_data);
		bool get_ssv_msg_data(can_data_t &can_data);
    bool get_ssrv_msg_data(can_data_t &can_data);
    bool get_sse_msg_data(can_data_t &can_data);
		bool handle_messages(can_data_t &can_data);
	public:
		Can_shared_data();
		bool get_message(Can_app_message& can_app_message);
		bool process_message(const Can_app_message& can_app_message);
};
