/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 21:05:09 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/25 01:48:13 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "shared_param/shared_param.hpp"
#include "queue/queue.hpp"
#include "shared_buffer/shared_buffer.hpp"

#include "../hdrs/test.hpp"

class Shared_data
{
  protected:

    enum e_state
    {
      SSV_MSG_REQUEST,
      SSRV_MSG_REQUEST,
      SSE_MSG_REQUEST,
      SYNCED,
      IS_ERR_STATE
    };
    
    static const uint16_t SSV_ALL_PERIOD = 500;
    static const uint8_t  COUNT = NUM_SYNC_PARAM;
    static const uint8_t  SSV_PERIOD = 5;
    static const uint8_t  SSRV_PERIOD = 2;
    static const uint8_t  SSE_PERIOD = 5;
    static const uint8_t  SSRV_ATTEMPTS = 3;
    static const uint8_t  SSRV_WAIT_TICKS = 25;
    static const uint8_t  TICK = 20;
    static const uint8_t  PACK_SIZE = 8;
    static const uint8_t  MIN_ACT_TICK = TICK * SSV_PERIOD;
    static const uint8_t  QUEUE_SIZE = COUNT;
    static const uint8_t  SHARED_PARM_MAX_DATA_LEN = 4;
    static const uint8_t  MAX_MESSAGE_SIZE = 8;

    struct sse_service_t
    {
      uint8_t counter;
      uint8_t idx;

      sse_service_t(uint8_t idx_val = csl_cmp_int<uint16_t>::not_valid(),
                    uint8_t counter_val = csl_cmp_int<uint8_t>::not_valid())
                    : counter(counter_val), idx(idx_val) {}
    };


    struct ssv_message_t
    {
      uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];
      uint16_t  iterator;
      uint16_t  param_num;

      ssv_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
    };

    struct ssrv_message_t
    {
      uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];
      uint16_t  param_num;

      ssrv_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
    };

    struct sse_message_t
    {
      uint16_t param_num;
      uint8_t  error_code;

      sse_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
    };
    
    bool get_ssv_message(ssv_message_t &message);
    bool get_ssrv_message(ssrv_message_t &message);
    bool get_sse_message(sse_message_t& message);
    bool handle_ssv_message(const ssv_message_t &message, const uint16_t id, const uint16_t id_can);
    bool handle_ssrv_message(const ssrv_message_t &message);
    bool handle_sse_message(const sse_message_t& message);

  private:
    Shared_param                      shared_params[COUNT];
    Shared_buffer                     shared_buffer;
    FSQueue<sse_service_t, PACK_SIZE> sse_queue;
    FSQueue<uint8_t, COUNT>           ssrv_queue;
    uint16_t                          idx_ssv;
    uint16_t                          idx_ssv_new;
    uint16_t                          tick;
    uint8_t                           state;

    bool  check_counter_ssv() const;
    uint16_t check_ssrv_wait_counter();
    bool  check_ssrv_new_value();
    uint16_t get_all_comm_obj_len(const uint16_t comm_obj_idx = COUNT) const;
    uint8_t get_sync_param_list_idx(const uint16_t p_num) const;
    void ssrv_time_management(const uint16_t ssrv_idx);
    void service_ssv();
    void service_ssrv();
    void service_sse();
    void service_shared_param();
    void service_wrn_state();
    void set_msg_request();
    void service_sync_state();
    bool  write_ssv_data(const uint16_t idx, ssv_message_t &message);
    inline uint16_t get_param_co_num(const uint16_t sync_param_idx) const
    {
      return sync_param_list[sync_param_idx];
    }
    inline void period_counter() { ++tick; }

  public:
    Shared_data();
    bool add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val);
    void init();
    void service();
    inline bool is_synced() const { return Bit::test(state, SYNCED); }
    inline bool is_ssv_msg_request() const { return Bit::test(state, SSV_MSG_REQUEST); }
    inline bool is_ssrv_msg_request() const { return Bit::test(state, SSRV_MSG_REQUEST); }
    inline bool is_sse_msg_request() const { return Bit::test(state, SSE_MSG_REQUEST); }
 };

#endif
