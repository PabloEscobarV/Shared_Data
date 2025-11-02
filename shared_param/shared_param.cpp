/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 21:02:52 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/11/02 21:35:50 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "shared_param.hpp"

#include "../p_iterator/p_iterator.hpp"
#include "../param_data/param_data.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace std;

const float Shared_param::FLOAT_PRECISION = 0.00001f;

Shared_param::Shared_param() : state(0)
{
  
}

bool Shared_param::accept_new_value(const uint16_t co_num, Shared_buffer& shared_buffer)
{
  bool result = is_new_value_allowed();

  if (result)
  {
    result = write_param_value(co_num, shared_buffer);
    update_iterator();
    reset_out_of_range_ssv_state();
    Bit::set(state, ACCEPTED_NEW_VALUE);
  }
  reset_counter();
  reset_new_value_not_allowed_state();
  return result;
}

bool Shared_param::check_wait_counter(const uint16_t current_tick) const
{
  if (is_new_val_wait_state())
  {
    return P_Iterator::get_simply_diff<int16_t, uint16_t>(current_tick, counter) >= MAX_WAIT_TICKS;
  }
  return false;
}

bool Shared_param::add_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, const uint8_t *ptr_new_param_val)
{
  bool result = check_new_value(co_num, ptr_new_param_val);

  if (result)
  {
    (void)shared_buffer.write_data(ptr_new_param_val, sizeof(uint32_t));
    reset_out_of_range_ssrv_state();
    reset_new_value_not_allowed_state();
    result = true;
  }
  return result;
}

bool Shared_param::get_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, uint8_t *ptr_data) const
{
  bool result = false;
  int param_len = sizeof(uint32_t);

  if (ptr_data)
  {
    result = shared_buffer.read_data(ptr_data, param_len);
  }
  return result;
}

bool Shared_param::get_param_value(const uint16_t co_num, uint8_t *ptr_dest) const
{
  uint32_t value = param_data->get_param_value(param_data->get_param_idx(co_num));
  
  if (value == ParamData::invalid_param_value())
  {
    return false;
  }
  memcpy(ptr_dest, &value, sizeof(uint32_t));
  return true;
}

bool Shared_param::handle_ssv_value(const uint16_t co_num,
                                    const uint8_t *ptr_param_val,
                                    const uint16_t iter_synchro,
                                    const bool id_can_id_less)
{
  const bool result = ptr_param_val && is_param_val_in_range(co_num, ptr_param_val);

  // cout << "Handling SSV Shared Param Value..."  << "IN RANGE: " << (result ? "YES" : "NO") << endl;
  if (result)
  {
    if (update_ssv_data(co_num, ptr_param_val, iter_synchro, id_can_id_less))
    {
      reset_out_of_range_ssv_state();
    }
  }
  else
  {
    set_out_of_range_ssv_state();
    reset_out_of_range_ssv_reset_state();
  }
  Bit::set(state, SYNCED);
  return result;
}

bool Shared_param::handle_ssrv_value(const uint16_t co_num, const uint8_t *ptr_new_p_val)
{
  const bool result = ptr_new_p_val && is_param_val_in_range(co_num, ptr_new_p_val);

  if (!result)
  {
    set_out_of_range_ssrv_state();
  }
  else
  {
    reset_out_of_range_ssrv_state();
  }
  return result;
}

bool Shared_param::handle_error_code(const uint8_t error_code)
{
  if (is_out_of_range_ssv_state(error_code))
  {
    // nothing specified yet.
  }
  if ((is_new_val_send_state() || is_new_val_wait_state()))
  {
    if (is_out_of_range_ssrv_state(error_code))
    {
      set_new_value_not_allowed_state();
    }
  }
  return true;
}

void Shared_param::reset_counter()
{
  counter = 0;
  reset_new_val_send_state();
  reset_new_val_wait_state();
}

void Shared_param::set_send_counter(const uint16_t cnt)
{
  counter = cnt;
  set_new_val_send_state();
}

void Shared_param::set_wait_time_stamp(const uint16_t time_stmp)
{
  counter = time_stmp;
  reset_new_val_send_state();
  set_new_val_wait_state();
}

void Shared_param::service(const uint16_t co_num,
                          Shared_buffer& shared_buffer,
                          const uint16_t current_tick,
                          const uint16_t check_flags_period)
{
  if ((current_tick != 0) && (current_tick % check_flags_period == 0))
  {
    service_flags();
  }
  service_new_value(co_num, current_tick, shared_buffer);
}

bool Shared_param::check_new_value(const uint16_t co_num, const uint8_t *ptr_new_param_val) const
{
  return (ptr_new_param_val != nullptr)
          && (!is_new_val_send_state())
          && (!is_new_val_wait_state())
          && is_param_val_in_range(co_num, ptr_new_param_val);
}

bool Shared_param::is_data_new(const uint16_t co_num, const uint8_t *ptr_new_param_value) const
{
  bool result = false;
  uint32_t new_value = 0;

  if (ptr_new_param_value)
  {
    memcpy(&new_value, ptr_new_param_value, sizeof(uint32_t));
    result = (new_value != param_data->get_param_value(param_data->get_param_idx(co_num)));
    // cout << "New Value: " << new_value << " | Current Value: "
    //      << param_data->get_param_value(param_data->get_param_idx(co_num)) 
    //      << " | Is Data New: " << result << endl;
  }
  return result;
}

bool Shared_param::is_update_allowed(const uint16_t iter_synchro, const bool is_local_id_less)
{
  bool is_req = true;

  if (P_Iterator::check_left_iter_is_newer(iterator.get_iterator(), static_cast<uint8_t>(iter_synchro)))
  {
    is_req = false;
  }
  if (is_req && is_local_id_less)
  {
    is_req = false;
  }
  return is_req;
}

bool Shared_param::is_param_val_in_range(const uint16_t co_num, const uint8_t *ptr_new_data) const
{
  bool in_range = true;
  uint32_t new_value = 0;

  if (ptr_new_data != nullptr)
  {
    (void)memcpy(&new_value, ptr_new_data, sizeof(uint32_t));
    in_range = new_value >= param_data->get_param_min_value(co_num)
               && new_value <= param_data->get_param_max_value(co_num);
  }
  return in_range;
}

void Shared_param::service_flags()
{
  reset_out_of_range_ssrv_state();
  if (is_out_of_range_ssv_reset_state())
  {
    reset_out_of_range_ssv_state();
  }
  else
  {
    set_out_of_range_ssv_reset_state();
  }
}

void Shared_param::service_new_value(const uint16_t co_num, const uint16_t current_tick, Shared_buffer& shared_buffer)
{
  if (check_wait_counter(current_tick))
  {
    accept_new_value(param_data->get_param_num(counter), shared_buffer);
  }
}

bool Shared_param::update_ssv_data(const uint16_t co_num,
                                  const uint8_t *ptr_param_val,
                                  const uint16_t iter_synchro,
                                  const bool id_can_id_less)
{
  bool result = iterator.update_iterator(static_cast<uint8_t>(iter_synchro));

  if(!result)
  {
    result = is_update_allowed(iter_synchro, id_can_id_less);
  }
  if (result)
  {
    result = write_param_value(co_num, ptr_param_val);
  }
  return result;
}

bool Shared_param::write_param_value(const uint16_t co_num, const uint8_t *ptr_new_param_value) const
{
  bool result = true;
  uint32_t new_data = 0;

  // cout << "Writing new param value..." << endl;
  if (is_data_new(co_num, ptr_new_param_value))
  {
    memcpy(&new_data, ptr_new_param_value, sizeof(uint32_t));
    param_data->set_param_value(param_data->get_param_idx(co_num), co_num, new_data);
  }
  return result;
}

bool Shared_param::write_param_value(const uint16_t co_num, Shared_buffer& shared_buffer) const
{
  bool result = false;
  uint32_t ptr_new_data_buff = 0;

  // cout << "Writing new param value 1..." << endl;
  if (shared_buffer.read_data(reinterpret_cast<uint8_t *>(&ptr_new_data_buff), sizeof(uint32_t)))
  {
    if (is_data_new(co_num, reinterpret_cast<uint8_t *>(&ptr_new_data_buff)))
      param_data->set_param_value(param_data->get_param_idx(co_num), co_num, ptr_new_data_buff);
  }
  return result;
}
