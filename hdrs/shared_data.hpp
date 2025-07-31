/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:54:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/31 15:27:33 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "shared_param.hpp"
#include "queue.hpp"
#include "test.hpp"
#include "time_stampt.hpp"

#include <cstdint>
#include <cstring>

#include <iostream>
#include <unistd.h>

using namespace std;

// TEMPORARY DECLARETION
uint16_t get_can_addr();

// TEMPORARY FUNCTION IMPLEMENTATION
uint16_t get_can_addr()
{
  return 0;
}

/*******************************************************************************************************************
 *  @struct can_data_t
 *  @brief CAN data structure for message transmission.
 *
 *  Structure that holds message data for CAN communication including
 *  message type, data payload, and indexing information.
 *******************************************************************************************************************/
struct can_data_t
{
  uint8_t  message_type;                 ///< Message type identifier
  uint8_t  data[8];  ///< Message data payload
  uint16_t data_len;                     ///< Length of data payload
  uint16_t idx;                          ///< Current iterator index
  uint16_t idx_can;                      ///< CAN-specific iterator index
};

/*********************************************************************************************************************
 *  @class SharedData
 *  @brief Template class for managing shared parameters with CAN communication.
 *
 *  This class manages a collection of shared parameters in a distributed system,
 *  providing CAN-based communication for parameter synchronization. It inherits
 *  from Can_data_handler_ifc to integrate with the CAN communication framework
 *  and handles SSV, SSRV, and SSE message types for comprehensive parameter management.
 *
 *  @tparam count Number of shared parameters to manage.
 ********************************************************************************************************************/

template <uint16_t count>
class SharedData : public Can_data_handler_ifc
{
  public:

    /*******************************************************************************************************************
     *  @enum message_type_t
     *  @brief Message type enumeration for CAN data identification.
     *
     *  Defines the different types of messages used in the shared data system.
     *******************************************************************************************************************/
    enum message_type_t
    {
      SSV_MESSAGE = 1,    ///< Set Shared Value message
      SSRV_MESSAGE,       ///< Set Shared Request Value message
      SSE_MESSAGE,        ///< Set Shared Error message
    };
    /*******************************************************************************************************************
     *  @brief Constructor for SharedData.
     *
     *  @param[in] ptr_can_app_ic Pointer to CAN application interface.
     *  @param[in] addr_name Controller address name.
     *
     *  Initializes the shared data system with CAN communication capabilities.
     *******************************************************************************************************************/
    SharedData(Can_app_ic_notify_new_data_ifc *ptr_can_app_ic, Controller_addresses::Values addr_name);

    /*******************************************************************************************************************
     *  @brief Store data from CAN thread.
     *
     *  @param[in] data_idx Index of data type.
     *  @param[in] ptr_src_data Pointer to source data.
     *  @param[in] src_data_len Length of source data.
     *  @param[in] can_addr CAN address.
     *
     *  @return true if data was stored successfully.
     *
     *  Handles incoming CAN messages and processes them according to message type.
     *******************************************************************************************************************/
    virtual bool set_data_can_thread(uint8_t data_idx,
                                     uint8_t *ptr_src_data,
                                     uint8_t src_data_len,
                                     uint8_t can_addr) OVERRIDE;

    /*******************************************************************************************************************
     *  @brief Get data for CAN thread.
     *
     *  @param[in] data_idx Index of data type.
     *  @param[out] ptr_dst_data Pointer to destination buffer.
     *  @param[in] dst_data_len Length of destination buffer.
     *  @param[in] addr_name Controller address name.
     *
     *  @return Length of data copied or NO_DATA if no data available.
     *
     *  Provides data for CAN transmission based on message type.
     *******************************************************************************************************************/
    virtual int8_t get_data_can_thread(uint8_t data_idx,
                                       uint8_t *ptr_dst_data,
                                       size_t dst_data_len,
                                       Controller_addresses::Values addr_name) OVERRIDE;

    /*******************************************************************************************************************
     *  @brief Get data for APP thread.
     *
     *  @param[in] data_idx Index of data type.
     *  @param[out] ptr_dst_data Pointer to destination buffer.
     *  @param[in] dst_data_len Length of destination buffer.
     *  @param[in] can_addr CAN address.
     *
     *  @return Length of data copied or NO_DATA if no data available.
     *
     *  Provides data for APP thread based on message type.
     *******************************************************************************************************************/
    virtual int8_t get_data_app_thread(uint8_t data_idx,
                                       uint8_t *ptr_dst_data,
                                       size_t dst_data_len,
                                       uint8_t can_addr) OVERRIDE;

    /*******************************************************************************************************************
     *  @brief Get data length for specified index.
     *
     *  @param[in] data_idx Index of data type.
     *
     *  @return Length of data for the specified index or NO_DATA if invalid.
     *
     *  Returns the expected data length for different message types.
     *******************************************************************************************************************/
    virtual int8_t get_data_len(uint8_t data_idx) const OVERRIDE;

    /*******************************************************************************************************************
     *  @brief Initialize the shared data system.
     *
     *  Performs initialization required by the Can_data_handler_ifc interface.
     *******************************************************************************************************************/
    virtual void initialize();

    /*******************************************************************************************************************
     *  @brief Initialize the shared data system.
     *
     *  Performs initialization required by the Can_data_handler_ifc interface.
     *******************************************************************************************************************/
    void init(const uint16_t params = 0, const uint16_t idx = 0);

    /*******************************************************************************************************************
     *  @brief Periodic processing counter.
     *
     *  Performs periodic tasks for parameter synchronization and message processing.
     *  Should be called regularly to maintain system operation.
     *******************************************************************************************************************/
    void period_counter();

    /*******************************************************************************************************************
     *  @brief Add SSRV message for parameter value change request.
     *
     *  @param[in] param_num Parameter number to change.
     *  @param[in] new_param_val New parameter value to request.
     *
     *  @return true if message was added successfully.
     *
     *  Adds a new parameter value change request to the processing queue.
     *******************************************************************************************************************/
    bool add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val, uint16_t size = SHARED_PARM_MAX_DATA_LEN);

    /*******************************************************************************************************************
     *  @brief Get messages for CAN transmission.
     *
     *  @param[out] can_data Reference to store message data.
     *
     *  @return true if message was retrieved successfully.
     *
     *  Retrieves pending messages for transmission over CAN bus.
     *******************************************************************************************************************/
    bool get_messages(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Handle incoming CAN messages.
     *
     *  @param[in] can_data Message data to process.
     *
     *  @return true if message was handled successfully.
     *
     *  Processes incoming CAN messages and updates parameter states accordingly.
     *******************************************************************************************************************/
    bool handle_messages(can_data_t &can_data);

  private:
    /*******************************************************************************************************************
     *  @struct ssrv_service_t
     *  @brief Service structure for SSRV message processing.
     *******************************************************************************************************************/
    struct ssrv_service_t
    {
      uint8_t idx;  ///< Parameter index
    };

    /*******************************************************************************************************************
     *  @struct sse_service_t
     *  @brief Service structure for SSE message processing.
     *******************************************************************************************************************/
    struct sse_service_t
    {
      uint8_t counter;  ///< Error counter
      uint8_t idx;      ///< Parameter index
    };

/*******************************************************************************************************************
 *  @struct ssv_message_t
 *  @brief Set Shared Value message structure.
 *
 *  Message structure for transmitting parameter value updates with sequence control.
 *  Used to synchronize parameter values across distributed nodes with conflict resolution.
 *******************************************************************************************************************/

 #include "pack_struct_begin.h"
 struct ssv_message_t
{
  uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];  ///< Parameter value to set
  uint16_t  iterator;   ///< Sequence iterator for message ordering
  uint16_t  param_num;  ///< Parameter number identifier
};
#include "pack_struct_end.h"

/*******************************************************************************************************************
 *  @struct ssrv_message_t
 *  @brief Set Shared Request Value message structure.
 *
 *  Packed message structure for requesting parameter value changes.
 *  Used for parameter modification requests that require validation.
 *******************************************************************************************************************/
#include "pack_struct_begin.h"
 struct ssrv_message_t
{
  uint16_t  param_num;  ///< Parameter number identifier
  uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];  ///< Requested parameter value
};
#include "pack_struct_end.h"

/*******************************************************************************************************************
 *  @struct sse_message_t
 *  @brief Set Shared Error message structure.
 *
 *  Packed message structure for transmitting error notifications related to parameters.
 *  Used to communicate parameter validation failures or access restrictions.
 *******************************************************************************************************************/
#include "pack_struct_begin.h"
 struct sse_message_t
{
  uint16_t param_num;  ///< Parameter number that caused the error
  uint8_t  error_code; ///< Error code identifier
};
#include "pack_struct_end.h"

    static const uint16_t SSV_ALL_PERIOD = 500;        ///< SSV all message period
    static const uint8_t QUEUE_SIZE = count / 5 + 1;  ///< Queue size calculation
    static const uint8_t SSV_PERIOD = 5;              ///< SSV message period
    static const uint8_t SSRV_PERIOD = 2;             ///< SSRV message period
    static const uint8_t SSE_PERIOD = 5;              ///< SSE message period
    static const uint8_t SSRV_ATTEMPTS = 3;           ///< SSRV attempt count
    static const uint8_t SSRV_WAIT_TICKS = 25;        ///< SSRV wait time

    FSQueue<sse_service_t, QUEUE_SIZE> sse_queue;      ///< SSE service queue
    FSQueue<ssrv_service_t, QUEUE_SIZE> ssrv_queue;    ///< SSRV service queue
    SharedParam *shared_params;                        ///< Array of shared parameters
    uint16_t   idx_ssv;                                ///< SSV index counter
    uint8_t    tick;                                   ///< Periodic counter

    /*******************************************************************************************************************
     *  @brief Get SSV message for transmission.
     *
     *  @param[out] message Reference to store SSV message.
     *
     *  @return true if message was generated successfully.
     *******************************************************************************************************************/
    bool get_ssv_message(ssv_message_t &message);

    /*******************************************************************************************************************
     *  @brief Get SSRV message for transmission.
     *
     *  @param[out] message Reference to store SSRV message.
     *
     *  @return true if message was generated successfully.
     *******************************************************************************************************************/
    bool get_ssrv_message(ssrv_message_t &message);

    /*******************************************************************************************************************
     *  @brief Get SSE message for transmission.
     *
     *  @param[out] message Reference to store SSE message.
     *
     *  @return true if message was generated successfully.
     *******************************************************************************************************************/
    bool get_sse_message(sse_message_t& message);

    /*******************************************************************************************************************
     *  @brief Set SSV message to CAN data structure.
     *
     *  @param[out] can_data Reference to store CAN data.
     *
     *  @return true if message was set successfully.
     *******************************************************************************************************************/
    bool set_ssv_message(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Set SSRV message to CAN data structure.
     *
     *  @param[out] can_data Reference to store CAN data.
     *
     *  @return true if message was set successfully.
     *******************************************************************************************************************/
    bool set_ssrv_message(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Set SSE message to CAN data structure.
     *
     *  @param[out] can_data Reference to store CAN data.
     *
     *  @return true if message was set successfully.
     *******************************************************************************************************************/
    bool set_sse_message(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSV message.
     *
     *  @param[in] message SSV message to process.
     *  @param[in] id Current iterator ID.
     *  @param[in] id_can CAN iterator ID.
     *
     *  @return true if message was handled successfully.
     *******************************************************************************************************************/
    bool handle_ssv_message(const ssv_message_t &message, const uint16_t id, const uint16_t id_can);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSRV message.
     *
     *  @param[in] message SSRV message to process.
     *
     *  @return true if message was handled successfully.
     *******************************************************************************************************************/
    bool handle_ssrv_message(const ssrv_message_t &message);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSE message.
     *
     *  @param[in] message SSE message to process.
     *
     *  @return true if message was handled successfully.
     *******************************************************************************************************************/
    bool handle_sse_message(const sse_message_t& message);

    /*******************************************************************************************************************
     *  @brief Get parameter number by index.
     *
     *  @param[in] idx Parameter index.
     *
     *  @return Parameter number at the specified index.
     *
     *  Retrieves the parameter number for the parameter at the given index.
     *******************************************************************************************************************/
    uint16_t  get_param_num(const uint16_t idx) const;

    /*******************************************************************************************************************
     *  @brief Get parameter index by parameter number.
     *
     *  @param[in] p_num Parameter number.
     *
     *  @return Parameter index.
     *******************************************************************************************************************/
    uint8_t get_idx(const uint16_t p_num) const;

    /*******************************************************************************************************************
     *  @brief Check SSRV end counters for timeout handling.
     *
     *  @return Number of parameters with expired counters.
     *******************************************************************************************************************/
    uint16_t check_ssrv_end_counters();

    /*******************************************************************************************************************
     *  @brief Check SSRV end counters for timeout handling.
     *
     *  @return true if it time to send SSV message.
     *  @return false if not.
     *******************************************************************************************************************/
    bool  check_counter_ssv() const;

    /*******************************************************************************************************************
     *  @brief Get ticks difference for time calculations.
     *
     *  @param[in] ticks_stamp Timestamp to compare with current tick.
     *
     *  @return Difference in ticks (including wraparound).
     *******************************************************************************************************************/
    inline int8_t get_ticks_diff(const uint8_t ticks_stamp) const
    {
      return static_cast<int8_t>(static_cast<int16_t>(tick - ticks_stamp));
    }

    static void shell_sort(SharedParam *array, const uint16_t size);
};

uint16_t get_step(uint16_t size);

/* ===================================================================================================================
 *  Template method implementations
 * ================================================================================================================ */

template <uint16_t count>
SharedData<count>::SharedData(Can_app_ic_notify_new_data_ifc * ptr_can_app_ic, Controller_addresses::Values addr_name)
  : Can_data_handler_ifc(ptr_can_app_ic, addr_name),
  idx_ssv(0),
  tick(0)
{
  shared_params = new(csl_new_assert, comap::IAllocator::get<comap::heap_CAN>()) SharedParam[count];
}

template <uint16_t count>
bool SharedData<count>::set_data_can_thread(uint8_t data_idx,
                                             uint8_t *ptr_src_data,
                                             uint8_t src_data_len,
                                             uint8_t can_addr)
{
  bool result = false;
  can_data_t can_data;

  if (src_data_len <= 8)
  {
    can_data.message_type = data_idx;
    can_data.data_len = src_data_len;
    can_data.idx = get_can_addr(); // temporary function (waiting on Tomas implementation)
    can_data.idx_can = can_addr;
    memcpy(can_data.data, ptr_src_data, src_data_len);
    result = handle_messages(can_data);
  }
  return result;
}

template <uint16_t count>
int8_t SharedData<count>::get_data_app_thread(uint8_t data_idx,
                                               uint8_t *ptr_dst_data,
                                               size_t dst_data_len,
                                               uint8_t can_addr)
{
  (void)data_idx;       // Suppress unused parameter warning
  (void)dst_data_len;   // Suppress unused parameter warning
  (void)can_addr;       // Suppress unused parameter warning
  (void)ptr_dst_data;   // Suppress unused parameter warning

  return Can_data_handler_ifc::NO_DATA; // No data for APP thread in this implementation
}

template <uint16_t count>
int8_t SharedData<count>::get_data_can_thread(uint8_t data_idx,
                                               uint8_t *ptr_dst_data,
                                               size_t dst_data_len,
                                               Controller_addresses::Values addr_name)
{
  can_data_t can_data;

  (void)data_idx;      // Suppress unused parameter warning
  (void)addr_name;      // Suppress unused parameter warning

  can_data.data_len = csl_cmp_int<uint8_t>::NOT_VALID;
  if (get_messages(can_data))
  {
    if (dst_data_len >= can_data.data_len)
    {
      memcpy(ptr_dst_data, can_data.data, can_data.data_len);
    }
  }
  return static_cast<int8_t>(can_data.data_len);
}

template <uint16_t count>
int8_t SharedData<count>::get_data_len(uint8_t data_idx) const
{
  int8_t result = NO_DATA;

  switch (data_idx)
  {
    case 0: // temporery idx for SSV message
      result = static_cast<int8_t>(sizeof(ssv_message_t));
      break;
    case 1: // temporery idx for SSRV message
      result = static_cast<int8_t>(sizeof(ssrv_message_t));
      break;
    case 2: // temporery idx for SSE message
      result = static_cast<int8_t>(sizeof(sse_message_t));
      break;
    default:
      break;
  }
  return result;
}

template <uint16_t count>
void SharedData<count>::initialize()
{
  for (int i = 0; i < NUM_SYNC_PARAM; i++)
  {
    shared_params[i].init(sync_param_list[i]);
  }
}

template <uint16_t count>
void  SharedData<count>::init(const uint16_t p_num, const uint16_t idx)
{
  if ((p_num > 0) && (idx < count))
  {
    shared_params[idx].init(p_num);
  }
}

template <uint16_t count>
uint16_t SharedData<count>::get_param_num(const uint16_t idx) const
{
  uint16_t p_num = 0;

  if (idx < count)
  {
    p_num = shared_params[idx].get_param_num();
  }
  return p_num;
}

template <uint16_t count>
void  SharedData<count>::period_counter()
{
  tick++;
}

template <uint16_t count>
bool  SharedData<count>::add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val, uint16_t size)
{
  ssrv_service_t new_message;
  bool  result = false;

  new_message.idx = get_idx(param_num);
  if (new_message.idx < count)
  {
    shared_params[new_message.idx].add_new_param_value(SSRV_ATTEMPTS, new_param_val, size);
    result = ssrv_queue.push(new_message);
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::get_messages(can_data_t &can_data)
{
  bool  result = false;

  result = set_sse_message(can_data);
  if (!result)
  {
    result = set_ssrv_message(can_data);
  }
  if (!result)
  {
    result = set_ssv_message(can_data);
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::handle_messages(can_data_t &can_data)
{
  ssv_message_t    ssv_message;
  ssrv_message_t  ssrv_message;
  sse_message_t    sse_message;
  bool  result = false;

  switch (can_data.message_type)
  {
    case SSV_MESSAGE:
      memcpy(&ssv_message, can_data.data, can_data.data_len);
      result = handle_ssv_message(ssv_message, can_data.idx, can_data.idx_can);
      break;
    case SSRV_MESSAGE:
      memcpy(&ssrv_message, can_data.data, can_data.data_len);
      result = handle_ssrv_message(ssrv_message);
      break;
    case SSE_MESSAGE:
      memcpy(&sse_message, can_data.data, can_data.data_len);
      result = handle_sse_message(sse_message);
      break;
    default:
      break;
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::get_ssv_message(ssv_message_t &message)
{
  uint16_t  idx = check_ssrv_end_counters();

  if (idx < count)
  {
    shared_params[idx].accept_new_value();
  }
  else
  {
    idx = idx_ssv;
    idx_ssv = static_cast<uint16_t>((idx_ssv + 1) % count);
  }
  message.iterator = shared_params[idx].get_iterator();
  message.param_num = shared_params[idx].get_param_num();
  return shared_params[idx].get_param_value(message.param_val);
}

template <uint16_t count>
bool  SharedData<count>::get_ssrv_message(ssrv_message_t &message)
{
  uint8_t  queue_counter = 0;
  ssrv_service_t  ssrv_service;
  const bool  result = ssrv_queue.pop(ssrv_service);

  if (result)
  {
    shared_params[ssrv_service.idx].get_ssrv_end_counter(queue_counter);
    message.param_num = shared_params[ssrv_service.idx].get_param_num();
    shared_params[ssrv_service.idx].get_param_value(message.param_val);
    if ((--queue_counter) > 0)
    {
      ssrv_queue.push(ssrv_service);
      ssrv_queue.swap(SSRV_PERIOD);
      shared_params[ssrv_service.idx].set_ssrv_end_counter(queue_counter);
    }
    else
    {
      shared_params[ssrv_service.idx].set_ssrv_end_counter(SSRV_WAIT_TICKS);
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::get_sse_message(sse_message_t &message)
{
  sse_service_t  sse_service;
  const bool  result = sse_queue.pop(sse_service);

  if (result)
  {
    message.param_num = shared_params[sse_service.idx].get_param_num();
    message.error_code = shared_params[sse_service.idx].get_error_code();
    sse_service.counter--;
    if (sse_service.counter > 0)
    {
      sse_queue.push(sse_service);
      sse_queue.swap(SSE_PERIOD);
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::set_ssv_message(can_data_t &can_data)
{
  ssv_message_t  message;
  bool  result = false;

  if (sizeof(ssv_message_t) <= can_data.data_len)
  {
    if (check_counter_ssv() && get_ssv_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(ssv_message_t));
      can_data.data_len = sizeof(ssv_message_t);
      can_data.message_type = SSV_MESSAGE;
      result = true;
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::set_ssrv_message(can_data_t &can_data)
{
  ssrv_message_t  message;
  bool  result = false;

  if (sizeof(ssrv_message_t) <= can_data.data_len)
  {
    if ((tick % SSRV_PERIOD == 0) && get_ssrv_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(ssrv_message_t));
      can_data.data_len = sizeof(ssrv_message_t);
      can_data.message_type = SSRV_MESSAGE;
      result = true;
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::set_sse_message(can_data_t &can_data)
{
  sse_message_t  message;
  bool  result = false;

  if ((sizeof(sse_message_t) <= can_data.data_len))
  {
    if (get_sse_message(message))
    {
      memcpy(&can_data.data, &message, sizeof(sse_message_t));
      can_data.data_len = sizeof(sse_message_t);
      can_data.message_type = SSE_MESSAGE;
      result = true;
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::handle_ssv_message(const ssv_message_t &message,
                                            const uint16_t id,
                                            const uint16_t id_can)
{
  sse_service_t  sse_service;
  bool  result = false;

  sse_service.counter = SSRV_ATTEMPTS;
  sse_service.idx = get_idx(message.param_num);
  if (sse_service.idx < count)
  {
    result = shared_params[sse_service.idx].handle_p_synchro(message.param_val, message.iterator, id, id_can);
    if (!result)
    {
      sse_queue.push(sse_service);
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::handle_ssrv_message(const ssrv_message_t &message)
{
  sse_service_t  sse_service;
  bool  result = false;

  sse_service.counter = 1;
  sse_service.idx = get_idx(message.param_num);
  if (sse_service.idx < count)
  {
    result = shared_params[sse_service.idx].handle_p_val_change_req(message.param_val);
    if (!result)
    {
      sse_queue.push(sse_service);
    }
  }
  return result;
}

template <uint16_t count>
bool  SharedData<count>::handle_sse_message(const sse_message_t &message)
{
  bool  result = false;
  const uint16_t idx = get_idx(message.param_num);

  if (idx < count)
  {
    result = shared_params[idx].handle_error_code(message.error_code);
  }
  return result;
}

template <uint16_t count>
uint8_t  SharedData<count>::get_idx(const uint16_t p_num) const
{
  uint8_t  left = 0;
  uint8_t  right = count;
  uint8_t   mid = 0;

  while (left < right)
  {
    mid = left + static_cast<uint8_t>((right - left) / 2);

    if (shared_params[mid].get_param_num() == p_num)
    {
      break ;
    }
    if (shared_params[mid].get_param_num() < p_num)
    {
      left = mid + 1;
    }
    else
    {
      right = mid;
    }
  }
  if (shared_params[mid].get_param_num() != p_num)
  {
    mid = count;
  }
  return mid;
}

template <uint16_t count>
uint16_t  SharedData<count>::check_ssrv_end_counters()
{
  uint8_t    ssrv_counter = 0;
  uint16_t  idx = 0;

  while (idx < count)
  {
    if (shared_params[idx].get_ssrv_end_counter(ssrv_counter))
    {
      if (get_ticks_diff(ssrv_counter) >= SSRV_WAIT_TICKS)
      {
        break ;
      }
    }
    ++idx;
  }
  return idx;
}

template <uint16_t count>
bool  SharedData<count>::check_counter_ssv() const
{
  bool  result = false;

  if (tick % SSV_PERIOD == 0)
  {
    result = true;
    if (idx_ssv == 0 && (tick % SSV_ALL_PERIOD != 0))
    {
      result = false;
    }
  }
  return result;
}

uint16_t get_step(uint16_t size)
{
	uint16_t step = 0;
	uint16_t max_step = size / 3;

	while (step < max_step)
	{
		step = static_cast<uint16_t>(step * 3 + 1);
	}
	return step;
}

template <uint16_t count>
void SharedData<count>::shell_sort(SharedParam *array, const uint16_t size)
{
	uint16_t tmp = 0;
	uint16_t j = 0;
	uint16_t step = get_step(size);

	while (step > 0)
	{
		for (uint16_t i = step; i < size; ++i)
		{
			tmp = array[i].get_param_num();
			j = i;
			while (j >= step && array[j - step].get_param_num() > tmp)
			{
				array[j] = array[j - step];
				j -= step;
			}
			array[j] = tmp;
		}
		step = (step - 1) / 3;
	}
}

// extern SharedData<P_COUNT> *shared_data;

#endif