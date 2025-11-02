/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 20:54:04 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/11/02 21:36:20 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_PARAM_HPP
#define SHARED_PARAM_HPP

#include "../p_iterator/p_iterator.hpp"
#include "../shared_buffer/shared_buffer.hpp"
#include "../bit/bit.hpp"

#include <stdint.h>

class Shared_param
{
    enum e_number_diff
    {
      CMP_ERROR = -2,
      FIRST_LESS,
      EQUAL,
      FIRST_GREATER
    };

    struct setpoint_limits_t
    {
      uint32_t low_limit;
      uint32_t high_limit;
    };

    static const float    FLOAT_PRECISION;
    static const uint8_t  SSRV_INCR_VALUE = 3;
    static const uint8_t  MAX_WAIT_TICKS = 25;

    uint16_t    counter;
    P_Iterator  iterator;
    uint8_t     state;

    bool accept_new_value(const uint16_t co_num, Shared_buffer& shared_buffer);
    bool check_new_value(const uint16_t co_num, const uint8_t *ptr_new_param_val) const;
    bool check_wait_counter(const uint16_t current_tick) const;
    template<typename data_t>
    int16_t cmp_data_with_type(const data_t a, const data_t b) const;
    bool is_data_new(const uint16_t co_num, const uint8_t *ptr_new_param_value) const;
    bool is_req_update_param_value(const uint16_t iter_synchro, const uint16_t idx, const uint16_t idx_can);
    bool is_update_allowed(const uint16_t iter_synchro, const bool is_local_id_less);
    bool is_param_val_in_range(const uint16_t co_num, const uint8_t *ptr_new_data) const;
    void notify_app_param_val(const uint16_t co_num) const;
    void service_flags();
    void service_new_value(const uint16_t co_num, const uint16_t current_tick, Shared_buffer& shared_buffer);
    bool update_ssv_data(const uint16_t co_num,
                        const uint8_t *ptr_param_val,
                        const uint16_t iter_synchro,
                        const bool id_can_id_less);
    bool write_param_value(const uint16_t co_num, const uint8_t *ptr_new_param_value) const;
    bool write_param_value(const uint16_t co_num, Shared_buffer& shared_buffer) const;
    inline bool is_out_of_range_ssv_reset_state() const
    {
      return Bit::test(state, OUT_OF_RANGE_SSV_RESET);
    }
    inline void update_iterator()
    {
      iterator += SSRV_INCR_VALUE + 1;
    }
    inline void set_new_value_not_allowed_state()
    {
      Bit::set(state, NEW_VAL_REQ_NOT_ALLOWED);
    }
    inline void reset_new_value_not_allowed_state()
    {
      Bit::clear(state, NEW_VAL_REQ_NOT_ALLOWED);
    }
    inline void set_out_of_range_ssv_state()
    {
      Bit::set(state, OUT_OF_RANGE_SSV);
    }
    inline void set_out_of_range_ssv_reset_state()
    {
      Bit::set(state, OUT_OF_RANGE_SSV_RESET);
    }
    inline void reset_out_of_range_ssv_state()
    {
      Bit::clear(state, OUT_OF_RANGE_SSV);
    }
    inline void reset_out_of_range_ssv_reset_state()
    {
      Bit::clear(state, OUT_OF_RANGE_SSV_RESET);
    }
    inline void set_out_of_range_ssrv_state()
    {
      Bit::set(state, OUT_OF_RANGE_SSRV);
    }
    inline void reset_out_of_range_ssrv_state()
    {
      Bit::clear(state, OUT_OF_RANGE_SSRV);
    }
    inline void set_new_val_send_state()
    {
      Bit::set(state, NEW_VAL_SEND_STATE);
    }
    inline void reset_new_val_send_state()
    {
      Bit::clear(state, NEW_VAL_SEND_STATE);
    }
    inline void set_new_val_wait_state()
    {
      Bit::set(state, NEW_VAL_WAIT_STATE);
    }
    inline void reset_new_val_wait_state()
    {
      Bit::clear(state, NEW_VAL_WAIT_STATE);
    }
  public:

    enum e_errorcode
    {
      OUT_OF_RANGE_SSV,
      OUT_OF_RANGE_SSV_RESET,
      OUT_OF_RANGE_SSRV,
      NEW_VAL_REQ_NOT_ALLOWED,
      NEW_VAL_SEND_STATE,
      NEW_VAL_WAIT_STATE,
      ACCEPTED_NEW_VALUE,
      SYNCED
    };

    static const uint16_t SHARED_PARM_MAX_DATA_LEN = 4;

    Shared_param();

    bool add_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, const uint8_t *ptr_new_param_val);
    bool get_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, uint8_t *ptr_data) const;
    bool get_param_value(const uint16_t co_num, uint8_t* dest) const;
    bool handle_ssv_value(const uint16_t co_num,
                          const uint8_t *param_val,
                          const uint16_t iter_synchro,
                          const bool id_can_id_less);
    bool handle_ssrv_value(const uint16_t co_num, const uint8_t *new_p_val);
    bool handle_error_code(const uint8_t error_code);
    void reset_counter();
    void set_send_counter(const uint16_t cnt);
    void set_wait_time_stamp(const uint16_t time_stmp);
    void service(const uint16_t co_num,
                Shared_buffer& shared_buffer,
                const uint16_t current_tick,
                const uint16_t check_flags_period);
    inline void decr_counter() { --counter; }
    inline uint16_t get_counter() const { return counter; }
    inline uint8_t get_error_code() const { return state; }
    inline uint16_t get_iterator() const { return iterator.get_iterator(); }
    inline void incr_counter() { ++counter; }
    inline bool is_new_val_wait_state() const { return Bit::test(state, NEW_VAL_WAIT_STATE); }
    inline bool is_new_val_send_state() const { return Bit::test(state, NEW_VAL_SEND_STATE); }
    inline bool is_new_value_allowed() const { return !Bit::test(state, NEW_VAL_REQ_NOT_ALLOWED); }
    inline bool is_synced() const { return Bit::test(state, SYNCED); }

    inline bool is_new_value_accepted()
    {
      const bool result = Bit::test(state, ACCEPTED_NEW_VALUE);

      Bit::clear(state, ACCEPTED_NEW_VALUE);
      return result;
    }

    inline bool is_out_of_range_ssv_state(const uint8_t error_code = UINT8_MAX) const
    {
      if (error_code == UINT8_MAX)
      {
        return Bit::test(state, OUT_OF_RANGE_SSV);
      }
      return Bit::test(error_code, OUT_OF_RANGE_SSV);
    }

    inline bool is_out_of_range_ssrv_state(const uint8_t error_code = UINT8_MAX) const
    {
      if (error_code == UINT8_MAX)
      {
        return Bit::test(state, OUT_OF_RANGE_SSRV);
      }
      return Bit::test(error_code, OUT_OF_RANGE_SSRV);
    }
    
    inline uint16_t send_iterator()
    {
      ++iterator;
      return iterator.get_iterator();
    }

};

#endif // SHARED_PARAM_HPP
