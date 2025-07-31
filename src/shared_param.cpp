/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:45:02 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/31 15:26:18 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_param.hpp"
#include "../hdrs/bit_operations.hpp"

#include "../hdrs/client_server_shared_setpoint.hpp"

#include <iostream>
#include <mutex>
#include <unistd.h>
#include <cstring>

using std::cout;
using std::endl;

namespace hide
{
/*******************************************************************************************************************
 *  @brief Static P_Iterator instance for internal use.
 *
 *  Due Renesas compiler error.
 *******************************************************************************************************************/
  static P_Iterator iter_hide;
} // hide

/* see header file */
SharedParam::SharedParam(const uint16_t param_idx) : iterator(&hide::iter_hide),
                                                    p_idx(param_idx)
{

}

/* see header file */
void SharedParam::init(const uint16_t param_idx)
{
  p_idx = param_idx;
}

/* see header file */
uint16_t SharedParam::get_iterator() const
{
  uint16_t iter = iterator->get_iterator();

  ++(*iterator);
  return iter;
}

/* see header file */
bool SharedParam::get_new_value(uint8_t *data, uint16_t size) const
{
  bool result = false;

  if (size <= SHARED_PARM_MAX_DATA_LEN)
  {
     memcpy(data, new_param_value, size);
     result = true;
  }
  return result;
}

/* see header file */
uint8_t SharedParam::get_error_code() const
{
  return err_code;
}

/* see header file */
bool SharedParam::handle_p_synchro(const uint8_t *param_val,
                                  const uint16_t iter_synchro,
                                  const uint16_t idx,
                                  const uint16_t idx_can)
{
  const bool result = is_param_value_ok(param_val);

  if (result)
  {
    if (is_req_update_param_value(iter_synchro, idx, idx_can))
    {
      write_param_value(param_val);
    }
  }
  else
  {
    comap::Bit::set(err_code, OUT_OF_RANGE_SSV);
  }
  return result;
}

/* see header file */
bool SharedParam::handle_p_val_change_req(const uint8_t *new_p_val)
{
  const bool result = is_param_value_ok(new_p_val);

  if (!result)
  {
    comap::Bit::set(err_code, OUT_OF_RANGE_SSRV);
  }
  return result;
}

/* see header file */
bool SharedParam::handle_error_code(const uint8_t error_code)
{
  if (comap::Bit::test(error_code, OUT_OF_RANGE_SSV))
  {

  }
  if (comap::Bit::test(err_code, SET_SSRV_COUNTER) && comap::Bit::test(error_code, OUT_OF_RANGE_SSRV))
  {
    comap::Bit::set(err_code, NEW_VAL_REQ_NOT_ALLOWED);
  }
  return true;
}

/* see header file */
bool SharedParam::accept_new_value()
{
  const bool result = !comap::Bit::test(err_code, NEW_VAL_REQ_NOT_ALLOWED);

  if (result)
  {
    write_param_value(new_param_value);
    update_iterator();
  }
  reset_ssrv_end_counter();
  return result;
}

/* see header file */
bool SharedParam::add_new_param_value(uint8_t ssrv_atmp_counter, uint8_t *new_param_val, uint16_t size)
{
  const bool result = !comap::Bit::test(err_code, SET_SSRV_COUNTER);

  if (result)
  {
    memcpy(new_param_value, new_param_val, size);
    set_ssrv_end_counter(ssrv_atmp_counter);
  }
  return result;
}

/* see header file */
bool SharedParam::is_req_update_param_value(const uint16_t iter_synchro,
                                            const uint16_t idx,
                                            const uint16_t idx_can)
{
  bool is_req = true;

  if (!iterator->update_iterator(iter_synchro))
  {
    if (P_Iterator::check_left_iter_is_newer(iterator->get_iterator(), iter_synchro))
    {
      is_req = false;
    }
    if (is_req && (idx_can > idx))
    {
      is_req = false;
    }
  }
  return is_req;
}

/* see header file */
void SharedParam::set_ssrv_end_counter(uint8_t counter)
{
  ssrv_counter = counter;
  comap::Bit::set(err_code, SET_SSRV_COUNTER);
}

/* see header file */
void SharedParam::reset_ssrv_end_counter()
{
  comap::Bit::clear(err_code, SET_SSRV_COUNTER);
}

/* see header file */
bool SharedParam::get_ssrv_end_counter(uint8_t& counter) const
{
  counter = ssrv_counter;
  return comap::Bit::test(err_code, SET_SSRV_COUNTER);
}

/* see header file */
bool SharedParam::is_new_value_allowed() const
{
  return !comap::Bit::test(err_code, NEW_VAL_REQ_NOT_ALLOWED);
}

/* see header file */
void SharedParam::update_iterator()
{
  *iterator += SSRV_INCR_VALUE;
}

/* see header file */
bool SharedParam::get_param_value(uint8_t *dest) const
{
  bool  result = false;
  co_descr_t descr;

  if ((app_comm_obj_get_descr(get_param_num(), &descr) == CO_DEF) && (descr.type == CO_SPAR))
  {
     if (csl_app_comm_obj_read(&descr, dest, SHARED_PARM_MAX_DATA_LEN))
     {
        result = true;
     }
  }
  return result;
}

/* see header file */
bool SharedParam::is_param_value_ok(const uint8_t *p_value, uint16_t size) const
{
  int64_t value = CheckParamRange::get_value_from_arr(p_idx, p_value, size);

  return (CheckParamRange::is_setpoint_val_out_of_range(p_idx, value));
}

/* see header file */
void SharedParam::write_param_value(const uint8_t *p_value, uint16_t size)
{
  co_descr_t descr;
  uint32_t received_value = 0;
  uint32_t actual_value = 0;
  uint16_t comm_obj = get_param_num();

  if ((app_comm_obj_get_descr(comm_obj, &descr) == CO_DEF) && (descr.type == CO_SPAR))
  {
    memcpy(&received_value, p_value, size);
    if (setpoint_read(descr.addr, descr.len, &actual_value))
    {
      if (actual_value != received_value)
      {
        set_param_value(descr, comm_obj, received_value);
      }
    }
  }
}

/* see header file */
void SharedParam::set_param_value(const co_descr_t& descr, const uint16_t& comm_obj, uint32_t& received_value)
{
  if (acodr_ok == sys_param_write_term(descr.addr, descr.len, &received_value))
  {
    notify_param_change(p_idx, Term_ts_flags::INVALID_TERM_ID, false);
    // notify app
    cb_notify_param_write(descr.addr, descr.len, UAM_NO_USER_SLOT, ID_SYNC_TERMINAL,
                          comm_obj, false, &received_value, true);
  }
}

/* see header file */
uint16_t SharedParam::get_param_num() const
{
  cfg_il3f_search_par_t search_par;

  cfg_get_search_par_item(p_idx, &search_par);
  return search_par.comm_obj;
}
