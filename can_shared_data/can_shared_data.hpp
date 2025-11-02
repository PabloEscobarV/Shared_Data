/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_shared_data.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 19:53:50 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/11/02 21:39:37 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAN_SHARED_DATA_HPP
#define CAN_SHARED_DATA_HPP

#include "../shared_data/shared_data.hpp"
#include "can_app_message.hpp"

#include <cstdint>

using namespace std;

class Can_shared_data : public Shared_data
{
    enum e_state
    {
      SYNCED,
      IS_MSG_REQUEST,
      IS_CFG_VALID
    };
    
    enum e_message_type
    {
      SSV_MESSAGE,
      SSRV_MESSAGE = 5,
      SSE_MESSAGE = 12,
    };

    enum e_ssv_msg_types
    {
      SSV_MESSAGE_0 = SSV_MESSAGE,
      SSV_MESSAGE_1,
      SSV_MESSAGE_2,
      SSV_MESSAGE_3,
      SSV_MESSAGE_4,
      SSV_MESSAGE_MAX_COUNT
    };

    enum e_ssrv_msg_types
    {
      SSRV_MESSAGE_0 = SSRV_MESSAGE,
      SSRV_MESSAGE_1,
      SSRV_MESSAGE_2,
      SSRV_MESSAGE_3,
      SSRV_MESSAGE_4,
      SSRV_MESSAGE_5,
      SSRV_MESSAGE_6,
      SSRV_MESSAGE_MAX_COUNT
    };

    enum e_sse_msg_types
    {
      SSE_MESSAGE_0 = SSE_MESSAGE,
      SSE_MESSAGE_1,
      SSE_MESSAGE_2,
      SSE_MESSAGE_3,
      SSE_MESSAGE_MAX_COUNT
    };

		static const uint8_t    MAX_DATA_LEN = 59;
		static const uint8_t    MAX_FLAGS_COUNT = 16;
    static const uint16_t   MSG_FLAGS[MAX_FLAGS_COUNT];

    struct can_data_t
    {
      uint8_t  data[MAX_DATA_LEN];              ///< Message data payload.
      uint8_t  messages_size[MAX_FLAGS_COUNT];  ///< Length of data payload.
      uint8_t  data_len;                        ///< Length of data payload.
      uint16_t idx_can;                         ///< CAN-specific iterator index.
      uint16_t offset;                          ///< Data offset for multi-message handling.
      uint16_t message_type_flags;              ///< Message type identifier.

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
      bool get_data(const uint16_t msg_type, data_t& dest);
    };

    uint32_t            _event;            ///< Event file descriptor for signaling.
		uint8_t	            _can_cu_id;        ///< Controller CAN address.
    uint8_t             _state;            ///< State of the class.

    bool is_cfg_valid() const
    {
      return Bit::test(_state, IS_CFG_VALID);
    }
		bool get_messages(can_data_t &can_data);
		bool get_ssv_msg_data(can_data_t &can_data);
    bool get_ssrv_msg_data(can_data_t &can_data);
    bool get_sse_msg_data(can_data_t &can_data);
		bool handle_messages(can_data_t &can_data);
    bool handle_ssv_msg_data(can_data_t &can_data);
    bool handle_ssrv_msg_data(can_data_t &can_data);
    bool handle_sse_msg_data(can_data_t &can_data);
    void set_msg_request();

    template <typename data_t>
    inline bool check_msg_req(const uint16_t current_msg, const uint16_t max_available_msgs, const uint16_t data_len) const
    {
      return (current_msg < max_available_msgs) && ((data_len + sizeof(data_t)) <= Can_shared_data::MAX_DATA_LEN);
    }

    inline bool check_ssv_msg_req(const uint16_t current_msg, const uint16_t data_len) const
    {
      return check_msg_req<ssv_message_t>(current_msg, SSV_MESSAGE_MAX_COUNT, data_len) && is_ssv_msg_request();
    }

	public:
		Can_shared_data();
		bool get_message(Can_app_message& can_app_message);
		bool process_message(const Can_app_message& can_app_message);
    void initialize(const uint16_t cu_id, int event_fd);
    void service();
    inline bool is_msg_request() const
    {
      return is_ssv_msg_request() || is_ssrv_msg_request() || is_sse_msg_request();
    }

    inline uint16_t get_iterator(const uint16_t sync_param_idx) const
    {
      return shared_params[sync_param_idx].get_iterator();
    }
};

#endif // CAN_SHARED_DATA_HPP