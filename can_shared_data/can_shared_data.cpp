/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   can_shared_data.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 21:25:06 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/26 13:05:15 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "can_shared_data.hpp"

#include "can_app_message.hpp"

#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

  const uint8_t Can_shared_data::MSG_FLAGS[Can_shared_data::MAX_FLAGS_COUNT] =
  {
    1 << Can_shared_data::SSV_MESSAGE_0,
    1 << Can_shared_data::SSV_MESSAGE_1,
    1 << Can_shared_data::SSV_MESSAGE_2,
    1 << Can_shared_data::SSV_MESSAGE_3,
    1 << Can_shared_data::SSV_MESSAGE_4,
    1 << Can_shared_data::SSRV_MESSAGE_0,
    1 << Can_shared_data::SSRV_MESSAGE_1,
    1 << Can_shared_data::SSRV_MESSAGE_2,
    1 << Can_shared_data::SSRV_MESSAGE_3,
    1 << Can_shared_data::SSRV_MESSAGE_4,
    1 << Can_shared_data::SSRV_MESSAGE_5,
    1 << Can_shared_data::SSRV_MESSAGE_6,
    1 << Can_shared_data::SSE_MESSAGE_0,
    1 << Can_shared_data::SSE_MESSAGE_1,
    1 << Can_shared_data::SSE_MESSAGE_2,
    1 << Can_shared_data::SSE_MESSAGE_3,
  };

Can_shared_data::can_data_t::can_data_t()
{
  memset(data, 0, sizeof(data));
  memset(messages_size, 0, sizeof(messages_size));
  data_len = 0;
  idx_can = 0;
  message_type_flags = 0;
}

Can_shared_data::can_data_t::can_data_t(const uint8_t *ptr_data,
                                    const uint8_t data_size,
                                    const uint16_t id_cu_received,
                                    const uint16_t message_flags)
  : data_len(data_size),
    idx_can(id_cu_received),
    message_type_flags(message_flags)
{
  memset(messages_size, 0, sizeof(messages_size));
  memset(data, 0, sizeof(data));
  if (ptr_data != nullptr)
  {
    memcpy(data, ptr_data, data_len);
  }
  else
  {
    data_len = 0;
  }
}

Can_shared_data::can_data_t::can_data_t(const uint8_t *ptr_data,
                                    const uint8_t (&msgs_size)[MAX_FLAGS_COUNT],
                                    const uint8_t data_size,
                                    const uint16_t id_cu_received,
                                    const uint16_t message_flags)
  : data_len(data_size),
    idx_can(id_cu_received),
    message_type_flags(message_flags)
{
  memcpy(messages_size, msgs_size, sizeof(msgs_size));
  memset(data, 0, sizeof(data));
  if (ptr_data != nullptr)
  {
    memcpy(data, ptr_data, data_len);
  }
  else
  {
    data_len = 0;
  }
}

template <typename data_t>
bool Can_shared_data::can_data_t::add_data(const data_t& data_obj,
                                      const uint16_t msg_type,
                                      const uint16_t msg_flag)
{
  bool result = false;

  if (((message_type_flags & msg_flag) == 0) && (msg_type < MAX_FLAGS_COUNT))
  {
    memcpy(data + data_len, &data_obj, sizeof(data_t));
    data_len += sizeof(data_t);
    messages_size[msg_type] = sizeof(data_t);
    message_type_flags |= msg_flag;
    result = true;
  }
  return result;
}

template <typename data_t>
data_t Can_shared_data::can_data_t::get_data(const uint16_t msg_type, const uint16_t msg_flag)
{
  data_t result;

  if (Bit::test(message_type_flags, msg_flag) && (messages_size[msg_type] == sizeof(data_t)))
  {
    offset = 0;
    for (uint16_t i = 0; i < msg_type; ++i)
    {
      offset += messages_size[i];
    }
  }
  memcpy(&result, data + offset, sizeof(data_t));
  return result;
}


void Can_shared_data::initialize(const uint16_t cu_id, int event_fd)
{
  can_cu_id = cu_id;
  event = event_fd;
  Bit::set(state, IS_CFG_VALID);
  Shared_data::initialize();
}

bool Can_shared_data::get_message(Can_app_message& can_app_message)
{
  can_data_t can_data;
  bool result = false;

  if (is_cfg_valid() && get_messages(can_data))
  {
    can_app_message = Can_app_message(can_data.data,
                                      can_data.data_len,
                                      Can_app_message::Can_app_message_type::Shared_data,
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

  if (is_ssv_msg_request())
  {
    result = get_ssv_msg_data(can_data);
  }
  if (result && is_sse_msg_request())
  {
    result &= get_sse_msg_data(can_data);
  }
  if (result && is_ssrv_msg_request())
  {
    result &= get_ssrv_msg_data(can_data);
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
  bool  result = false;

  for (uint8_t i = 0; ((can_data.data_len + sizeof(ssv_message_t)) <= MAX_DATA_LEN); ++i)
  {
    if (!get_ssv_message(message) || (SSV_MESSAGE_0 + i >= SSV_MESSAGE_MAX_COUNT))
    {
      break ;
    }
    result |= can_data.add_data(message, SSV_MESSAGE_0 + i, MSG_FLAGS[SSV_MESSAGE_0 + i]);
  }
  return result;
}

bool  Can_shared_data::get_ssrv_msg_data(can_data_t &can_data)
{
  ssrv_message_t  message;
  bool  result = false;

  for (uint8_t i = 0; ((can_data.data_len + sizeof(ssrv_message_t)) <= MAX_DATA_LEN); ++i)
  {
    if (!get_ssrv_message(message) || (SSRV_MESSAGE_0 + i >= SSRV_MESSAGE_MAX_COUNT))
    {
      break ;
    }
    result |= can_data.add_data(message, SSRV_MESSAGE_0 + i, MSG_FLAGS[SSRV_MESSAGE_0 + i]);
  }
  return result;
}

bool  Can_shared_data::get_sse_msg_data(can_data_t &can_data)
{
  sse_message_t  message;
  bool  result = get_sse_message(message);

  for (uint8_t i = 0; ((can_data.data_len + sizeof(sse_message_t)) <= MAX_DATA_LEN); ++i)
  {
    if (!get_sse_message(message) && (SSE_MESSAGE_0 + i >= SSE_MESSAGE_MAX_COUNT))
    {
      break ;
    }
    result |= can_data.add_data(message, SSE_MESSAGE_0 + i, MSG_FLAGS[SSE_MESSAGE_0 + i]);
  }
  return result;
}

bool  Can_shared_data::handle_messages(can_data_t &can_data)
{
  bool  result = false;

  if (Bit::test(can_data.message_type_flags, MSG_FLAGS[SSV_MESSAGE_0]))
  {
    result = handle_ssv_msg_data(can_data);
  }
  if (result && Bit::test(can_data.message_type_flags, MSG_FLAGS[SSRV_MESSAGE_0]))
  {
    result &= handle_ssrv_msg_data(can_data);
  }
  if (result && Bit::test(can_data.message_type_flags, MSG_FLAGS[SSE_MESSAGE_0]))
  {
    result &= handle_sse_msg_data(can_data);
  }
  return result;
}

bool Can_shared_data::handle_ssv_msg_data(can_data_t &can_data)
{
  bool result = false;

  for (uint8_t i = SSV_MESSAGE_0; i < SSV_MESSAGE_MAX_COUNT; ++i)
  {
    result &= handle_ssv_message(can_data.get_data<ssv_message_t>(i, MSG_FLAGS[i]), can_cu_id, can_data.idx_can);
  }
  return result;
}

bool Can_shared_data::handle_ssrv_msg_data(can_data_t &can_data)
{
  bool result = false;

  for (uint8_t i = SSRV_MESSAGE_0; i < SSRV_MESSAGE_MAX_COUNT; ++i)
  {
    result &= handle_ssrv_message(can_data.get_data<ssrv_message_t>(i, MSG_FLAGS[i]));
  }
  return result;
}

bool Can_shared_data::handle_sse_msg_data(can_data_t &can_data)
{
  bool result = false;

  for (uint8_t i = SSE_MESSAGE_0; i < SSE_MESSAGE_MAX_COUNT; ++i)
  {
    result &= handle_sse_message(can_data.get_data<sse_message_t>(i, MSG_FLAGS[i]));
  }
  return result;
}

void Can_shared_data::service()
{ 
  Shared_data::service();
  if (is_msg_request())
  {
    set_msg_request();
  }
}

void Can_shared_data::set_msg_request()
{
  bool event_request = true;
  
  write(event, &event_request, sizeof(event_request));
}
