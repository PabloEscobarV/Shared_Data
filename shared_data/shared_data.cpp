/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 21:11:03 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/25 02:01:31 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shared_data.hpp"
#include "../queue/queue.hpp"
#include "../shared_param/shared_param.hpp"
#include "../csl_cmp_int/csl_cmp_int.hpp"

#include <stdint.h>
#include <string.h>


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


Shared_data::Shared_data()
  : idx_ssv(0),
    idx_ssv_new(csl_cmp_int<uint16_t>::not_valid()),
    tick(0),
    state(0)
{

}

bool  Shared_data::add_ssrv_message(const uint16_t param_num, const uint8_t *ptr_new_param_val)
{
  uint8_t ssrv_idx;
  bool  result = false;

  if (Bit::test(state, SYNCED))
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

void Shared_data::init()
{
  shared_buffer.init(get_all_comm_obj_len());
}

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

bool  Shared_data::check_counter_ssv() const
{
  bool  result = true;

  if ((idx_ssv_new == csl_cmp_int<uint16_t>::not_valid()) && (tick > MIN_ACT_TICK))
  {
    if ((idx_ssv == 0) && (tick % SSV_ALL_PERIOD != 0))
    {
      result = false;
    }
  }
  return result;
}

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

uint16_t Shared_data::get_all_comm_obj_len(const uint16_t comm_obj_idx) const
{
  return sizeof(uint16_t) * NUM_SYNC_PARAM;
}

bool  Shared_data::get_ssv_message(ssv_message_t &message)
{
  bool result = false;

  if (idx_ssv_new == csl_cmp_int<uint16_t>::not_valid())
  {
    result = write_ssv_data(idx_ssv, message);
    idx_ssv = static_cast<uint16_t>((idx_ssv + 1) % COUNT);
  }
  else
  {
    result = write_ssv_data(idx_ssv_new, message);
    idx_ssv_new = csl_cmp_int<uint16_t>::not_valid();
  }
  Bit::clear(state, SSV_MSG_REQUEST);
  return result;
}

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
    Bit::clear(state, SSRV_MSG_REQUEST);
  }
  return result;
}

bool  Shared_data::get_sse_message(sse_message_t &message)
{
  sse_service_t  sse_service;
  const bool  result = sse_queue.pop(sse_service) && (sse_service.idx < COUNT);

  if (result)
  {
    message.param_num = get_param_co_num(sse_service.idx);
    message.error_code = shared_params[sse_service.idx].get_error_code();
    if (--sse_service.counter > 0)
    {
      sse_queue.push(sse_service, PACK_SIZE - 1);
    }
    Bit::clear(state, SSE_MSG_REQUEST);
  }
  return result;
}

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

bool  Shared_data::handle_ssv_message(const ssv_message_t &message,
                                            const uint16_t id,
                                            const uint16_t id_can)
{
  sse_service_t  sse_service(get_sync_param_list_idx(message.param_num), SSRV_ATTEMPTS);
  bool  result = false;

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

bool  Shared_data::handle_ssrv_message(const ssrv_message_t &message)
{
  sse_service_t  sse_service(get_sync_param_list_idx(message.param_num), 1);
  bool  result = false;

  if (sse_service.idx < COUNT)
  {
    result = shared_params[sse_service.idx].handle_ssrv_value(get_param_co_num(sse_service.idx), message.param_val);
    if (!result)
    {
      result = sse_queue.push(sse_service);
      Bit::set(state, SSE_MSG_REQUEST);
    }
  }
  return result;
}

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

void Shared_data::service_ssv()
{
  if (Bit::test(state, SYNCED) && check_counter_ssv())
  {
    if (check_ssrv_new_value() || ((tick != 0) && (tick % Shared_data::SSV_PERIOD == 0)))
    {
      Bit::set(state, SSV_MSG_REQUEST);
    }
  }
}

void Shared_data::service_ssrv()
{
  if ((!ssrv_queue.is_empty()) && (tick != 0) && (tick % Shared_data::SSRV_PERIOD == 0))
  {
    Bit::set(state, SSRV_MSG_REQUEST);
  }
}

void Shared_data::service_sse()
{
  if ((!sse_queue.is_empty()) && (tick != 0) && (tick % Shared_data::SSE_PERIOD == 0))
  {
    Bit::set(state, SSE_MSG_REQUEST);
  }
}

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
  Bit::set(state, IS_ERR_STATE);
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
