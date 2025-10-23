/*******************************************************************************************************************
 *  @file shared_data.cpp
 *  @brief Shared data management for distributed parameter synchronization via CAN.
 *
 *  @date Created: 2025/07/10 08:54:29
 *  @date Updated: 2025/07/21 14:26:02
 *
 *  @par Copyright (c) 2025 ComAp a.s  All rights reserved.
 *******************************************************************************************************************/

// own header
#include "shared_data.hpp"

// other includes
#include "addr_mngr.hpp"                        // Addr_mngr – addresses mapping helper
#include "comm.h"                               // NUM_SYNC_PARAM, sync_param_list
#include "csl_cmp_int.hpp"                      // csl_cmp_int – NOT_VALID constant and helpers
#include "can_app_ic_notify_new_data_ifc.hpp"   // Can_app_ic_notify_new_data_ifc – notify interface
#include "csl_new.hpp"                          // csl_new_assert
#include "../can_data_handler_ifc.hpp"          // Can_data_handler_ifc – base class interface
#include "iallocator.hpp"                       // IAllocator – memory allocator interface
#include "fixed_size_queue/queue.hpp"           // FSQueue – fixed-size queue for messages
#include "shared_param/shared_param.hpp"        // Shared_param – setpoint synchronization state machine

extern "C"
{
  #include "csl_states_history_ifc.h"           // states_serve_err_now
  #include "states_def.h"                       // idx_s_ ...
  #include "csl_printf.h"                       // csl_printf
}

// standard library includes
#include <stdint.h>
#include <string.h>

namespace comap
{

/* ================================================================================================================
 *  Private class type static members definition
 * ============================================================================================================== */

  const uint16_t Shared_data::SSRV_MSG_FLAGS[Shared_data::PACK_SIZE] =
  {
    1 << Shared_data::SSRV_MESSAGE_0,
    1 << Shared_data::SSRV_MESSAGE_1,
    1 << Shared_data::SSRV_MESSAGE_2,
    1 << Shared_data::SSRV_MESSAGE_3,
    1 << Shared_data::SSRV_MESSAGE_4,
    1 << Shared_data::SSRV_MESSAGE_5,
    1 << Shared_data::SSRV_MESSAGE_6,
    1 << Shared_data::SSRV_MESSAGE_7
  };

/* ================================================================================================================
 *  Private class type methods definition
 * ============================================================================================================== */

Shared_data::ssv_message_t::ssv_message_t(const uint8_t *ptr_data, const uint16_t data_len)
  : iterator(0),
    param_num(0)
{
  memset(param_val, 0, sizeof(param_val));
  if (ptr_data != nullptr && data_len == (sizeof(param_val) + 2 * sizeof(uint16_t)))
  {
    memcpy(param_val, ptr_data, sizeof(param_val));
    memcpy(&iterator, ptr_data + sizeof(param_val), sizeof(iterator));
    memcpy(&param_num, ptr_data + sizeof(param_val) + sizeof(iterator), sizeof(param_num));
  }
}

Shared_data::ssrv_message_t::ssrv_message_t(const uint8_t *ptr_data, const uint16_t data_len)
  : param_num(0)
{
  memset(param_val, 0, sizeof(param_val));
  if (ptr_data != nullptr && data_len == (sizeof(param_val) + sizeof(param_num)))
  {
    memcpy(param_val, ptr_data, sizeof(param_val));
    memcpy(&param_num, ptr_data + sizeof(param_val), sizeof(param_num));
  }
}

Shared_data::sse_message_t::sse_message_t(const uint8_t *ptr_data, const uint16_t data_len)
  : param_num(0),
    error_code(0)
{
  if ((ptr_data != nullptr) && (data_len == (sizeof(param_num) + sizeof(error_code))))
  {
    memcpy(&param_num, ptr_data, sizeof(param_num));
    memcpy(&error_code, ptr_data + sizeof(param_num), sizeof(error_code));
  }
}

Shared_data::can_data_t::can_data_t()
{
  memset(data, 0, sizeof(data));
  memset(messages_size, 0, sizeof(messages_size));
  data_len = 0;
  idx_can = 0;
  message_type_flags = 0;
}

Shared_data::can_data_t::can_data_t(const uint8_t *ptr_data,
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

Shared_data::can_data_t::can_data_t(const uint8_t *ptr_data,
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
bool Shared_data::can_data_t::add_data(const data_t& data_obj,
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
data_t Shared_data::can_data_t::get_data()
{
  data_t result;

  if (offset < data_len)
  {
    memcpy(&result, data + offset, sizeof(data_t));
    offset += sizeof(data_t);
  }
  return result;
}

/* ================================================================================================================
 *  Public class methods definition
 * ============================================================================================================== */

/* see header file */
Shared_data::Shared_data()
  : shared_data_event(0),
    ptr_thread(nullptr),
    idx_ssv(0),
    idx_ssv_new(csl_cmp_int<uint16_t>::NOT_VALID),
    tick(0),
    cu_id(csl_cmp_int<uint16_t>::NOT_VALID),
    state(0)
{

}

/* see header file */
bool  Shared_data::add_ssrv_message(const uint16_t param_num, const uint8_t *ptr_new_param_val)
{
  uint8_t ssrv_idx;
  bool  result = false;

  if (Bit::test(state, SYNCED)) // blocking on 10 sec. after import configuration.
  {
    ssrv_idx = get_sync_param_list_idx(param_num);
    if ((ssrv_idx < COUNT) && (shared_buffer.set_offset(get_all_comm_obj_len(ssrv_idx))))
    {
      if (shared_params[ssrv_idx].add_new_value(get_param_co_num(ssrv_idx),
                                                shared_buffer,
                                                ptr_new_param_val))
      {
        shared_params[ssrv_idx].set_send_counter(SSRV_ATTEMPTS);
        result = ssrv_queue.push(ssrv_idx);
      }
    }
  }
  return result;
}

/* see header file */
bool Shared_data::get_message(Can_app_message& can_app_message)
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
    // csl_nprintf(80,
    //     "%s%s%s %lu%s%d%s%s\n",
    //     DARK_RED,
    //     BOLD,
    //     __func__,
    //     __LINE__,
    //     "\n--- GET MSG: TYPE ",
    //     (int)can_app_message.get_type(),
    //     " ---\n",
    //     RESET_FORMAT);
  }
  return result;
}

/* see header file */
void Shared_data::initialize(cmsis::Cmsis_thread *thread_ptr, const uint8_t cu_can_address, const uint32_t event_mask)
{
  ptr_thread = thread_ptr;
  cu_id = cu_can_address;
  shared_data_event = event_mask;
  shared_buffer.init(get_all_comm_obj_len());
  if (cfg_valid())
  {
    Bit::set(state, IS_CFG_VALID);
  }
}

/* see header file */
bool Shared_data::process_message(const Can_app_message& can_app_message)
{
  bool result = false;
  can_data_t can_data(can_app_message.get_data_pointer(),
                      can_app_message.get_data_len(),
                      can_app_message.get_source_controller_id(),
                      can_app_message.get_flags());

  csl_nprintf(80,
    "%s%s%s %lu%s%s\n",
    DARK_RED,
    BOLD,
    __func__,
    __LINE__,
    "\n--- PROCCESS MSG ---\n",
    RESET_FORMAT);

  if (can_data.data && (can_data.data_len <= MAX_DATA_LEN) && is_cfg_valid())
  {
    result = handle_messages(can_data);
  }
  return result;
}

/* see header file */
void Shared_data::service()
{
  period_counter();
  service_sync_state();
  service_ssv();
  service_ssrv();
  service_sse();
  service_shared_param();
  service_wrn_state();
  set_msg_request();
}

/* see header file */
bool Shared_data::sync_param_are_synced()
{
  return Bit::test(state, SYNCED);
}

/* ================================================================================================================
 *  Private class methods definition
 * ============================================================================================================== */

/* see header file */
bool  Shared_data::check_counter_ssv() const
{
  bool  result = true;

  if ((idx_ssv_new == csl_cmp_int<uint16_t>::NOT_VALID) && (tick > MIN_ACT_TICK))
  {
    if ((idx_ssv == 0) && (tick % SSV_ALL_PERIOD != 0))
    {
      result = false;
    }
  }
  return result;
}

/* see header file */
uint16_t  Shared_data::check_ssrv_wait_counter()
{
  uint16_t  idx = 0;

  for (idx = 0; idx < COUNT; ++idx)
  {
    if (shared_params[idx].is_new_val_wait_state())
    {
      if (P_Iterator::get_simply_diff<int16_t, uint16_t>(tick, shared_params[idx].get_counter()) >= SSRV_WAIT_TICKS)
      {
        break;
      }
    }
  }
  return idx;
}

/* see header file */
bool Shared_data::check_ssrv_new_value()
{
  bool result = false;
  uint16_t idx = check_ssrv_wait_counter();

  result = (idx < COUNT)
          && shared_buffer.set_offset(get_all_comm_obj_len(idx))
          && shared_params[idx].accept_new_value(get_param_co_num(idx), shared_buffer);
  if (result)
  {
    idx_ssv_new = idx;
  }
  return result;
}

/* see header file */
uint16_t Shared_data::get_all_comm_obj_len(const uint16_t comm_obj_idx) const
{
  uint16_t total_length = 0;
  co_descr_t descr;

  for (uint16_t i = 0; i < COUNT && i < comm_obj_idx; ++i)
  {
    if ((app_comm_obj_get_descr(get_param_co_num(i), &descr) == CO_DEF) && (descr.type == CO_SPAR))
    {
      total_length += descr.len;
    }
  }
  return total_length;
}

/* see header file */
bool  Shared_data::get_messages(can_data_t &can_data)
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

/* see header file */
bool  Shared_data::get_ssv_message(ssv_message_t &message)
{
  bool result = false;

  // csl_nprintf(80,
  //   "%s%s%s %lu%s%s\n",
  //   DARK_RED,
  //   BOLD,
  //   __func__,
  //   __LINE__,
  //   "\n--- GET SSV MSG ---\n",
  //   RESET_FORMAT);
  if (idx_ssv_new == csl_cmp_int<uint16_t>::NOT_VALID)
  {
    result = write_ssv_data(idx_ssv, message);
    idx_ssv = static_cast<uint16_t>((idx_ssv + 1) % COUNT);
  }
  else
  {
    result = write_ssv_data(idx_ssv_new, message);
    idx_ssv_new = csl_cmp_int<uint16_t>::NOT_VALID;
  }
  return result;
}

/* see header file */
bool  Shared_data::get_ssrv_message(ssrv_message_t &message)
{
  uint8_t ssrv_idx;
  bool result = ssrv_queue.peek(ssrv_idx) && (ssrv_idx < COUNT);

  if (result)
  {
    message.param_num = get_param_co_num(ssrv_idx);
    result = shared_buffer.set_offset(get_all_comm_obj_len(ssrv_idx))
            && shared_params[ssrv_idx].get_new_value(message.param_num,
                                                    shared_buffer,
                                                    message.param_val);
    ssrv_time_management(ssrv_idx);
  }
  return result;
}

/* see header file */
bool  Shared_data::get_sse_message(sse_message_t &message)
{
  sse_service_t  sse_service;
  const bool  result = sse_queue.pop(sse_service);

  if (result && (sse_service.idx < COUNT))
  {
    message.param_num = get_param_co_num(sse_service.idx);
    message.error_code = shared_params[sse_service.idx].get_error_code();
    if (--sse_service.counter > 0)
    {
      sse_queue.push(sse_service, PACK_SIZE - 1);
    }
  }
  return result;
}

/* see header file */
bool  Shared_data::get_ssv_msg_data(can_data_t &can_data)
{
  ssv_message_t  message;
  bool  result = check_counter_ssv() && get_ssv_message(message);

  if (result)
  {
    result = can_data.add_data(message, SSV_MESSAGE, SSV_MSG_FLAG);
  }
  return result;
}

/* see header file */
bool  Shared_data::get_ssrv_msg_data(can_data_t &can_data)
{
  ssrv_message_t  message;
  bool  result = false;

  for (uint8_t i = 0; ((can_data.data_len + sizeof(ssrv_message_t)) <= MAX_DATA_LEN) && get_ssrv_message(message); ++i)
  {
    result |= can_data.add_data(message, SSRV_MESSAGE + i, SSRV_MSG_FLAGS[i]);
  }
  return result;
}

/* see header file */
bool  Shared_data::get_sse_msg_data(can_data_t &can_data)
{
  sse_message_t  message;
  bool  result = get_sse_message(message);

  if (result)
  {
    result = can_data.add_data(message, SSE_MESSAGE, SSE_MSG_FLAG);
  }
  return result;
}

/* see header file */
uint8_t  Shared_data::get_sync_param_list_idx(const uint16_t co_num) const
{
  uint8_t   left = 0;
  uint8_t   right = COUNT;
  uint8_t   mid = 0;

  while (left < right)
  {
    mid = left + static_cast<uint8_t>((right - left) / 2);
    if (sync_param_list[mid] == co_num)
    {
      break ;
    }
    if (sync_param_list[mid] < co_num)
    {
      left = mid + 1;
    }
    else
    {
      right = mid;
    }
  }
  if (sync_param_list[mid] != co_num)
  {
    mid = COUNT;
  }
  return mid;
}

/* see header file */
bool  Shared_data::handle_messages(can_data_t &can_data)
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

/* see header file */
bool  Shared_data::handle_ssv_message(const ssv_message_t &message,
                                            const uint16_t id,
                                            const uint16_t id_can)
{
  sse_service_t  sse_service;
  bool  result = false;

  csl_nprintf(80,
    "%s%s%s %lu%s%s\n",
    DARK_RED,
    BOLD,
    __func__,
    __LINE__,
    "\n--- HANDLE SSV MSG ---\n",
    RESET_FORMAT);

  sse_service.counter = SSRV_ATTEMPTS;
  sse_service.idx = get_sync_param_list_idx(message.param_num);
  if (sse_service.idx < COUNT)
  {
    result = shared_params[sse_service.idx].handle_ssv_value(
              get_param_co_num(sse_service.idx), message.param_val, message.iterator, id, id_can);
    if (!result)
    {
      result = sse_queue.push(sse_service);
    }
  }
  return result;
}

/* see header file */
bool  Shared_data::handle_ssrv_message(const ssrv_message_t &message)
{
  sse_service_t  sse_service;
  bool  result = false;

  sse_service.counter = 1;
  sse_service.idx = get_sync_param_list_idx(message.param_num);
  if (sse_service.idx < COUNT)
  {
    result = shared_params[sse_service.idx].handle_ssrv_value(get_param_co_num(sse_service.idx), message.param_val);
    if (!result)
    {
      result = sse_queue.push(sse_service);
      ptr_thread->set_signal(shared_data_event);
      Bit::set(state, SSE_MSG_REQUEST);
    }
  }
  return result;
}

/* see header file */
bool  Shared_data::handle_sse_message(const sse_message_t &message)
{
  bool  result = false;
  const uint16_t idx = get_sync_param_list_idx(message.param_num);

  if (idx < COUNT)
  {
    result = shared_params[idx].handle_error_code(message.error_code);
  }
  return result;
}

void Shared_data::ssrv_time_management(const uint16_t ssrv_idx)
{
  if (ssrv_idx < COUNT)
  {
    shared_params[ssrv_idx].decr_counter();
    if ((shared_params[ssrv_idx].is_new_val_send_state()) && (shared_params[ssrv_idx].get_counter() > 0))
    {
      ssrv_queue.swap(PACK_SIZE - 1);
    }
    else
    {
      ssrv_queue.remove();
      shared_params[ssrv_idx].set_wait_time_stamp(tick);
    }
  }
}

/* see header file */
void Shared_data::service_ssv()
{
  if (Bit::test(state, SYNCED)) // blocking on 10 sec. after import configuration.
  {
    if (check_ssrv_new_value() || ((tick != 0) && (tick % Shared_data::SSV_PERIOD == 0)))
    {
      Bit::set(state, SSV_MSG_REQUEST);
    }
  }
}

/* see header file */
void Shared_data::service_ssrv()
{
  if ((!ssrv_queue.is_empty()) && (tick != 0) && (tick % Shared_data::SSRV_PERIOD == 0))
  {
    Bit::set(state, SSRV_MSG_REQUEST);
  }
}

/* see header file */
void Shared_data::service_sse()
{
  if ((!sse_queue.is_empty()) && (tick != 0) && (tick % Shared_data::SSE_PERIOD == 0))
  {
    Bit::set(state, SSE_MSG_REQUEST);
  }
}

/* see header file */
void Shared_data::service_shared_param()
{
  if ((tick != 0) && (tick % Shared_data::SSV_ALL_PERIOD == 0))
  {
    for (uint16_t i = 0; i < COUNT; ++i)
    {
      shared_params[i].service();
    }
  }
}

/* see header file */
void Shared_data::service_wrn_state()
{
  bool is_err = false;

  for (uint16_t i = 0; i < COUNT; ++i)
  {
    if (shared_params[i].is_out_of_range_ssv_state() || !(shared_params[i].is_new_value_allowed()))
    {
      is_err = true;
      break;
    }
  }
  states_serve_err_now(is_err, false, idx_s_CommobjOutOfRange, lvls_s_CommobjOutOfRange);
}

void Shared_data::set_msg_request()
{
  if (Bit::test(state, SSV_MSG_REQUEST) || Bit::test(state, SSRV_MSG_REQUEST) || Bit::test(state, SSE_MSG_REQUEST))
  {
    ptr_thread->set_signal(shared_data_event);
  }
}

void Shared_data::service_sync_state()
{
  if (!Bit::test(state, SYNCED))
  {
    Bit::set(state, SYNCED);
    if (tick < SSV_ALL_PERIOD)
    {
      for (uint16_t i = 0; i < COUNT; ++i)
      {
        if (!shared_params[i].is_synced())
        {
          Bit::clear(state, SYNCED);
          break;
        }
      }
    }
  }
}

bool  Shared_data::write_ssv_data(const uint16_t idx, ssv_message_t &message)
{
  bool result = false;

  if (idx < COUNT)
  {
    message.iterator = shared_params[idx].send_iterator();
    message.param_num = get_param_co_num(idx);
    result = shared_params[idx].get_param_value(get_param_co_num(idx), message.param_val);
  }
  return result;
}

} // namespace comap
