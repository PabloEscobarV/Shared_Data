/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_shared_data.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 21:25:06 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/24 21:28:10 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "can_shared_data.hpp"

bool Can_shared_data::get_message(Can_app_message& can_app_message)
{
  can_data_t can_data;
  bool result = false;

  if (is_cfg_valid() && get_messages(can_data))
  {
    can_app_message = Can_app_message(can_data.data,
                                      can_data.data_len,
                                      Can_app_message_type::Shared_data,
                                      can_data.message_type_flags,
                                      can_app_message.get_source_controller_id(),
                                      can_app_message.get_sid(),
                                      can_data.messages_size
                                    );
    result = true;
  }
  return result;
}

bool  Can_shared_data::get_messages(can_data_t &can_data)
{
  bool  result = false;

  if (Bit::test(state, SSV_MSG_REQUEST))
  {
    result = get_ssv_msg_data(can_data);
    Bit::clear(state, SSV_MSG_REQUEST);
  }
  if (result && Bit::test(state, SSE_MSG_REQUEST))
  {
    result &= get_sse_msg_data(can_data);
    Bit::clear(state, SSE_MSG_REQUEST);
  }
  if (result && Bit::test(state, SSRV_MSG_REQUEST))
  {
    result &= get_ssrv_msg_data(can_data);
    Bit::clear(state, SSRV_MSG_REQUEST);
  }
  return result;
}

bool Can_shared_data::process_message(const Can_app_message& can_app_message)
{
  bool result = false;
  can_data_t can_data(can_app_message.get_data_pointer(),
                      can_app_message.get_data_len(),
                      can_app_message.get_source_controller_id(),
                      can_app_message.get_flags());

  if (can_data.data && (can_data.data_len <= MAX_DATA_LEN) && is_cfg_valid())
  {
    result = handle_messages(can_data);
  }
  return result;
}

bool  Can_shared_data::get_ssv_msg_data(can_data_t &can_data)
{
  ssv_message_t  message;
  bool  result = check_counter_ssv() && get_ssv_message(message);

  if (result)
  {
    result = can_data.add_data(message, SSV_MESSAGE, SSV_MSG_FLAG);
  }
  return result;
}

bool  Can_shared_data::get_ssrv_msg_data(can_data_t &can_data)
{
  ssrv_message_t  message;
  bool  result = false;

  for (uint8_t i = 0; ((can_data.data_len + sizeof(ssrv_message_t)) <= MAX_DATA_LEN) && get_ssrv_message(message); ++i)
  {
    result |= can_data.add_data(message, SSRV_MESSAGE + i, SSRV_MSG_FLAGS[i]);
  }
  return result;
}

bool  Can_shared_data::get_sse_msg_data(can_data_t &can_data)
{
  sse_message_t  message;
  bool  result = get_sse_message(message);

  if (result)
  {
    result = can_data.add_data(message, SSE_MESSAGE, SSE_MSG_FLAG);
  }
  return result;
}

bool  Can_shared_data::handle_messages(can_data_t &can_data)
{
  bool  result = false;

  if (can_data.message_type_flags & SSV_MSG_FLAG)
  {
    result = handle_ssv_message(can_data.get_data<ssv_message_t>(), cu_id, can_data.idx_can);
  }
  if (can_data.message_type_flags & SSE_MSG_FLAG)
  {
    result &= handle_sse_message(can_data.get_data<sse_message_t>());
  }
  for (uint8_t i = 0; ((i < PACK_SIZE) && (can_data.offset + sizeof(ssrv_message_t)) <= can_data.data_len); ++i)
  {
    if (can_data.message_type_flags & SSRV_MSG_FLAGS[i])
    {
      result &= handle_ssrv_message(can_data.get_data<ssrv_message_t>());
    }
  }
  return result;
}
