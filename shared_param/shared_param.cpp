/*******************************************************************************************************************
 *  @file shared_param.cpp
 *  @brief Implementation of Shared_param class for distributed parameter management.
 *
 *  @date Created: 2025/07/09 21:45:02
 *  @date Updated: 2025/07/21 10:29:28 by blackrider
 *******************************************************************************************************************/

// own header
#include "shared_param.hpp"

// other includes
#include "../p_iterator/p_iterator.hpp"                 // P_Iterator – iterator with wrap-around logic

#include "csl_cmp_int.hpp"                              // csl_cmp_int – NOT_VALID constant and helpers
#include "csl_bit.hpp"                                  // Bit operations utility
#include "term_ts_flags.hpp"                            // Term_ts_flags – terminal flags

extern "C"
{
  #include "app_comm_obj_descr.h"                       // app_comm_obj_get_descr
  #include "cfg_il3f_types.h"                           // cfg_il3f_types
  #include "csl_app_comm_obj.h"                         // csl_app_comm_obj
  #include "csl_setpoints_ifc.h"                        // csl_setpoint_read
  #include "cfg_access.h"                               // cfg_access
  #include "cfg_access_ifc.h"                           // cfg_get_descr_par_item
  #include "states_def.h"                               // idx_s_ ...
  #include "data_types.h"                               // data types
  #include "sys_callbacks.h"                            // cb_notify_param_write
  #include "sys_ifc.h"                                  // sys_param_write_term
  #include "stdfloat.h"                                 // for float32_t
  #include "terminal.h"                                 // notify_param_change
}

// standard library includes
#include <cstdint>                                      // Fixed-width integer types
#include <cstring>                                      // memcpy

namespace comap
{

const float32_t Shared_param::FLOAT_PRECISION = 0.00001f;      ///< Precision for floating-point comparisons.

/* ================================================================================================================
 *  Public class methods definition
 * ============================================================================================================== */

/* see header file */
Shared_param::Shared_param() : err_code(NO_ERROR)
{

}

/* see header file */
bool Shared_param::accept_new_value(const uint16_t co_num, Shared_buffer& shared_buffer)
{
  bool result = is_new_value_allowed();

  if (result && (Bit::test(err_code, SYNCED)))
  {
    result = write_param_value(co_num, shared_buffer);
    update_iterator();
    reset_out_of_range_ssv_state();
  }
  else
  {
    set_param_value(co_num, co_descr_t()); // Due IC do not update value of parameter.
  }
  reset_counter();
  return result;
}

/* see header file */
bool Shared_param::add_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, const uint8_t *ptr_new_param_val)
{
  bool result = false;
  co_descr_t descr;

  if (check_new_value(co_num, ptr_new_param_val, descr))
  {
    (void)shared_buffer.write_data(ptr_new_param_val, descr.len);
    reset_out_of_range_ssrv_state();
    reset_new_value_not_allowed_state();
    result = true;
  }
  return result;
}

/* see header file */
bool Shared_param::get_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, uint8_t *ptr_data) const
{
  bool result = false;
  co_descr_t descr;

  if (ptr_data && get_descr(co_num, descr))
  {
    result = shared_buffer.read_data(ptr_data, descr.len);
  }
  return result;
}

/* see header file */
bool Shared_param::get_param_value(const uint16_t co_num, uint8_t *ptr_dest) const
{
  co_descr_t descr;
  bool  result = false;

  if (ptr_dest && get_descr(co_num, descr) && (descr.len <= SHARED_PARM_MAX_DATA_LEN))
  {
    if (csl_app_comm_obj_read(&descr, ptr_dest, SHARED_PARM_MAX_DATA_LEN))
    {
      result = true;
    }
  }
  return result;
}

/* see header file */
bool Shared_param::handle_ssv_value(const uint16_t co_num,
                                    const uint8_t *ptr_param_val,
                                    const uint16_t iter_synchro,
                                    const uint16_t id_can,
                                    const uint16_t id_can_received)
{
  bool result = ptr_param_val && is_param_val_in_range(co_num, ptr_param_val);

  if (result)
  {
    Bit::set(err_code, SYNCED);
    if (is_req_update_param_value(iter_synchro, id_can, id_can_received) && write_param_value(co_num, ptr_param_val))
    {
      reset_out_of_range_ssv_state();
    }
  }
  else
  {
    set_out_of_range_ssv_state();
    reset_out_of_range_ssv_reset_state();
  }
  return result;
}

/* see header file */
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

/* see header file */
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

/* see header file */
void Shared_param::reset_counter()
{
  counter = 0;
  reset_new_val_send_state();
  reset_new_val_wait_state();
}

/* see header file */
void Shared_param::set_send_counter(const uint16_t cnt)
{
  counter = cnt;
  set_new_val_send_state();
}

/* see header file */
void Shared_param::set_wait_time_stamp(const uint16_t time_stmp)
{
  counter = time_stmp;
  reset_new_val_send_state();
  set_new_val_wait_state();
}

void Shared_param::service()
{
  reset_out_of_range_ssrv_state();
  reset_new_value_not_allowed_state();
  if (is_out_of_range_ssv_reset_state())
  {
    reset_out_of_range_ssv_state();
  }
  else
  {
    set_out_of_range_ssv_reset_state();
  }
}

/* ================================================================================================================
 *  Private class methods definition
 * ============================================================================================================== */

/* see header file */
bool Shared_param::check_new_value(const uint16_t co_num, const uint8_t *ptr_new_param_val, co_descr_t& descr) const
{
  return (ptr_new_param_val != nullptr)
          && (!is_new_val_send_state())
          && (!is_new_val_wait_state())
          && get_descr(co_num, descr)
          && is_param_val_in_range(co_num, ptr_new_param_val);
}

/* see header file */
template<typename data_t>
int16_t Shared_param::cmp_data_with_type(const data_t a, const data_t b) const
{
  if (a < b)
  {
    return FIRST_LESS;
  }
  if (a > b)
  {
    return FIRST_GREATER;
  }
  return EQUAL;
}

/* see header file */
template<>
int16_t Shared_param::cmp_data_with_type<float32_t>(const float32_t a, const float32_t b) const
{
  float32_t result = a - b;

  if (fabs(result) <= FLOAT_PRECISION)
  {
    return EQUAL;
  }
  if (result < 0.0f)
  {
    return FIRST_LESS;
  }
  return FIRST_GREATER;
}

/* see header file */
int16_t Shared_param::cmp_data_correct_type(const uint8_t* ptr_first_data,
                                            const uint8_t* ptr_second_data,
                                            const uint8_t data_type) const
{
  int16_t result = CMP_ERROR;

  switch (data_type)
  {
    case ctd_INTEGER8:
      result = set_cmp_data<int8_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_INTEGER16:
      result = set_cmp_data<int16_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_INTEGER32:
      result = set_cmp_data<int32_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_FLOAT:
      result = set_cmp_data<float32_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_UNSIGNED8:
    case ctd_BINARY8:
    case ctd_STRLIST:
    case ctd_CHAR:
      result = set_cmp_data<uint8_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_UNSIGNED16:
    case ctd_BINARY16:
      result = set_cmp_data<uint16_t>(ptr_first_data, ptr_second_data);
      break;
    case ctd_UNSIGNED32:
    case ctd_BINARY32:
      result = set_cmp_data<uint32_t>(ptr_first_data, ptr_second_data);
      break;
    default:
      break;
  }
  return result;
}

/* see header file */
bool Shared_param::get_descr(const uint16_t co_num, co_descr_t& descr) const
{
  bool result = false;

  if ((app_comm_obj_get_descr(co_num, &descr) == CO_DEF) && (descr.type == CO_SPAR))
  {
    result = true;
  }
  return result;
}

/* see header file */
Shared_param::setpoint_limits_t Shared_param::get_setpoint_limits(const cfg_il3f_descr_par_t& setpoint_description) const
{
  setpoint_limits_t limits;

  limits.low_limit = static_cast<uint32_t>(setpoint_description.low_limit);
  limits.high_limit = static_cast<uint32_t>(setpoint_description.high_limit);
  if (ctd_STRLIST == setpoint_description.type)
  {
    limits.low_limit = 0;
    limits.high_limit = limits.high_limit - limits.low_limit;
  }
  if (0 != setpoint_description.var_low_limit)
  {
    limits.low_limit = read_param_value(static_cast<uint16_t>(limits.low_limit));
  }
  if (0 != setpoint_description.var_high_limit)
  {
    limits.high_limit = read_param_value(static_cast<uint16_t>(limits.high_limit));
  }
  return limits;
}

 /* see header file */
bool Shared_param::is_data_new(const co_descr_t& descr, const uint8_t *ptr_new_param_value) const
{
  bool result = false;
  uint32_t actual_value = 0;

  if (ptr_new_param_value && setpoint_read(descr.addr, descr.len, &actual_value))
  {
    if (descr.len <= SHARED_PARM_MAX_DATA_LEN)
    {
      result = (memcmp(&actual_value, ptr_new_param_value, descr.len) != 0);
    }
  }
  return result;
}

/* see header file */
bool Shared_param::is_req_update_param_value(const uint16_t iter_synchro,
                                            const uint16_t id_can,
                                            const uint16_t id_can_received)
{
  bool is_req = true;

  if (!iterator.update_iterator(static_cast<uint8_t>(iter_synchro)))
  {
    if (P_Iterator::check_left_iter_is_newer(iterator.get_iterator(), static_cast<uint8_t>(iter_synchro)))
    {
      is_req = false;
    }
    if (is_req && (id_can_received > id_can))
    {
      is_req = false;
    }
  }
  return is_req;
}

/** see header file **/
bool Shared_param::is_param_val_in_range(const uint16_t co_num, const uint8_t *ptr_new_data) const
{
  bool in_range = true;
  cfg_il3f_descr_par_t setpoint_description;
  setpoint_limits_t limits;
  co_descr_t descr;
  const uint16_t param_cfg_idx = cfg_get_param_idx(co_num);

  if ((param_cfg_idx < cfg_get_num_par(pt_ALL)) && (ptr_new_data != nullptr))
  {
    get_descr(co_num, descr);
    cfg_get_descr_par_item(param_cfg_idx, &setpoint_description);
    limits = get_setpoint_limits(setpoint_description);
    in_range = cmp_data_correct_type(ptr_new_data,
              reinterpret_cast<uint8_t *>(&limits.low_limit), descr.data_type) != FIRST_LESS;
    in_range &= cmp_data_correct_type(ptr_new_data,
                reinterpret_cast<uint8_t *>(&limits.high_limit), descr.data_type) != FIRST_GREATER;
  }
  return in_range;
}

/* see header file */
uint32_t Shared_param::read_param_value(const uint16_t param_cfg_idx) const
{
  uint32_t value = 0;

  cfg_il3f_search_par_t search_par_item;
  cfg_get_search_par_item(param_cfg_idx, &search_par_item);
  get_param_value(search_par_item.comm_obj, reinterpret_cast<uint8_t *>(&value));
  return value;
}

template <typename data_t>
int16_t Shared_param::set_cmp_data(const uint8_t* ptr_first_data, const uint8_t* ptr_second_data) const
{
  data_t first_value = 0;
  data_t second_value = 0;

  (void)memcpy(&first_value, ptr_first_data, sizeof(data_t));
  (void)memcpy(&second_value, ptr_second_data, sizeof(data_t));
  return cmp_data_with_type<data_t>(first_value, second_value);
}

/* see header file */
bool Shared_param::set_param_value(const uint16_t co_num, const co_descr_t& descr, const uint8_t *ptr_param_value) const
{
  uint16_t param_cfg_idx = cfg_get_param_idx(co_num);
  bool result = (ptr_param_value != nullptr);

  if (param_cfg_idx < cfg_get_num_par(pt_ALL))
  {
    result = result && (acodr_ok == sys_param_write_term(descr.addr, descr.len, ptr_param_value));
    if (result)
    {
      cb_notify_param_write(descr.addr, descr.len, UAM_NO_USER_SLOT, ID_SYNC_TERMINAL,
                            co_num, false, const_cast<uint8_t *>(ptr_param_value), true);
    }
    notify_param_change(param_cfg_idx, Term_ts_flags::INVALID_TERM_ID, false);
  }
  return result;
}

/* see header file */
bool Shared_param::write_param_value(const uint16_t co_num, const uint8_t *ptr_new_param_value) const
{
  bool result = false;
  co_descr_t descr;

  if (get_descr(co_num, descr))
  {
    result = true;
    if (is_data_new(descr, ptr_new_param_value))
    {
      result = set_param_value(co_num, descr, ptr_new_param_value);
    }
  }
  return result;
}

/* see header file */
bool Shared_param::write_param_value(const uint16_t co_num, Shared_buffer& shared_buffer) const
{
  bool result = false;
  co_descr_t descr;
  uint32_t ptr_new_data_buff = 0;

  if (get_descr(co_num, descr) && shared_buffer.read_data(reinterpret_cast<uint8_t *>(&ptr_new_data_buff), descr.len))
  {
    result = set_param_value(co_num, descr, reinterpret_cast<uint8_t *>(&ptr_new_data_buff));
  }
  return result;
}

} // namespace comap
