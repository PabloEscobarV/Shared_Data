/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 21:05:09 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/23 21:13:33 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "shared_param/shared_param.hpp"
#include "fixed_size_queue/queue.hpp"
#include "shared_buffer/shared_buffer.hpp"

class Shared_data
{
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
    static const uint8_t  MAX_DATA_LEN = 59;
    static const uint8_t  MAX_FLAGS_COUNT = 16;
    static const uint8_t  MAX_MESSAGE_SIZE = 8;

    static const uint8_t  SSV_MSG_FLAG = 1 << SSV_MESSAGE;
    static const uint8_t  SSE_MSG_FLAG = 1 << SSE_MESSAGE;
    static const uint16_t SSRV_MSG_FLAGS[PACK_SIZE];

    struct sse_service_t
    {
      uint8_t counter;
      uint8_t idx;
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

    struct can_data_t
    {
      uint8_t  data[MAX_DATA_LEN];
      uint8_t  messages_size[MAX_FLAGS_COUNT];
      uint8_t  data_len;
      uint16_t idx_can;
      uint16_t offset;
      uint16_t message_type_flags;

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
      data_t get_data();
    };

    Shared_param                      shared_params[COUNT];
    Shared_buffer                     shared_buffer;
    FSQueue<sse_service_t, PACK_SIZE> sse_queue;
    FSQueue<uint8_t, COUNT>           ssrv_queue;
    uint32_t                          shared_data_event;
    cmsis::Cmsis_thread*              ptr_thread;
    uint16_t                          idx_ssv;
    uint16_t                          idx_ssv_new;
    uint16_t                          tick;
    uint16_t                          cu_id;
    uint8_t                           state;

    bool  check_counter_ssv() const;
    uint16_t check_ssrv_wait_counter();
    bool  check_ssrv_new_value();
    uint16_t get_all_comm_obj_len(const uint16_t comm_obj_idx = COUNT) const;
    bool get_messages(can_data_t &can_data);
    bool get_ssv_message(ssv_message_t &message);
    bool get_ssrv_message(ssrv_message_t &message);
    bool get_sse_message(sse_message_t& message);
    bool get_ssv_msg_data(can_data_t &can_data);
    bool get_ssrv_msg_data(can_data_t &can_data);
    bool get_sse_msg_data(can_data_t &can_data);
    uint8_t get_sync_param_list_idx(const uint16_t p_num) const;
    bool handle_messages(can_data_t &can_data);
    bool handle_ssv_message(const ssv_message_t &message, const uint16_t id, const uint16_t id_can);
    bool handle_ssrv_message(const ssrv_message_t &message);
    bool handle_sse_message(const sse_message_t& message);
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
      rt_assert((sync_param_idx < COUNT), "Bad index of sync_param_list");
      return sync_param_list[sync_param_idx];
    }
    inline bool is_cfg_valid() const
    {
      return Bit::test(state, IS_CFG_VALID);
    }
  public:

    enum e_message_type
    {
      SSV_MESSAGE,
      SSE_MESSAGE,
      SSRV_MESSAGE,
    };

    enum e_ssrv_messages
    {
      SSRV_MESSAGE_0 = SSRV_MESSAGE,
      SSRV_MESSAGE_1,
      SSRV_MESSAGE_2,
      SSRV_MESSAGE_3,
      SSRV_MESSAGE_4,
      SSRV_MESSAGE_5,
      SSRV_MESSAGE_6,
      SSRV_MESSAGE_7,
    };

    enum e_state
    {
      NO_STATE,
      SSV_MSG_REQUEST,
      SSRV_MSG_REQUEST,
      SSE_MSG_REQUEST,
      SYNCED,
      IS_CFG_VALID,
    };

    Shared_data();
    bool add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val);
    bool get_message(Can_app_message& can_app_message);
    void initialize(cmsis::Cmsis_thread *thread_ptr, const uint8_t cu_can_address, const uint32_t event_mask);
    bool process_message(const Can_app_message& can_app_message);
    void service();
    bool sync_param_are_synced();
    inline void period_counter() { ++tick; }

 };

#endif
