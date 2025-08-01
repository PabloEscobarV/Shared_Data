/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_data.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:54:29 by blackrider        #+#    #+#             */
/*   Updated: 2025/08/01 14:40:03 by blackrider       ###   ########.fr       */
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

class SharedData : public Can_data_handler_ifc
{
  public:

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

    /***************************************************************************************************************//**
     * @brief Do processing of periodic tasks. It should be called periodically (40 ms).
     *
     * @param active_controllers ... bit mask of active controllers
     *
     * @thread_safety_NO
     * @call_from_ISR_NO
     ******************************************************************************************************************/
    void service(uint64_t active_controllers);

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

    static const uint16_t SSV_ALL_PERIOD = 500;       ///< SSV all message period
    static const uint8_t COUNT = NUM_SYNC_PARAM;      ///< Number of shared parameters
    static const uint8_t QUEUE_SIZE = COUNT / 5 + 1;  ///< Queue size calculation
    static const uint8_t SSV_PERIOD = 5;              ///< SSV message period
    static const uint8_t SSRV_PERIOD = 2;             ///< SSRV message period
    static const uint8_t SSE_PERIOD = 5;              ///< SSE message period
    static const uint8_t SSRV_ATTEMPTS = 3;           ///< SSRV attempt count
    static const uint8_t SSRV_WAIT_TICKS = 25;        ///< SSRV wait time

    FSQueue<sse_service_t, QUEUE_SIZE> sse_queue;      ///< SSE service queue
    FSQueue<ssrv_service_t, QUEUE_SIZE> ssrv_queue;    ///< SSRV service queue
    SharedParam shared_params[COUNT];                  ///< Array of shared parameters
    uint16_t  idx_ssv;                                 ///< SSV index counter
    uint8_t   address_name;                            ///< CAN address
    uint8_t   tick;                                    ///< Periodic counter

   /*******************************************************************************************************************
     *  @brief Initialize the shared data system.
     *
     *  Performs initialization required by the Can_data_handler_ifc interface.
     *******************************************************************************************************************/
    void init(const uint16_t params = 0, const uint16_t idx = 0);

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
// extern SharedData<P_COUNT> *shared_data;

#endif