/*******************************************************************************************************************
 *  @file shared_data.hpp
 *  @brief Shared data management for distributed parameter synchronization via CAN.
 *
 *  @date Created: 2025/07/10 08:54:29
 *  @date Updated: 2025/07/21 14:26:02
 *
 *  @par Copyright (c) 2025 ComAp a.s  All rights reserved.
 *******************************************************************************************************************/

#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "shared_param/shared_param.hpp"
#include "fixed_size_queue/queue.hpp"
#include "shared_buffer/shared_buffer.hpp"

#include "can_data_handler_ifc.hpp"         // For Can_data_handler_ifc
#include "can_app_ic.hpp"                   // For cmi_SSV, cmi_SSRV, cmi_SSE
#include "comm.h"                           // NUM_SYNC_PARAM, sync_param_list
#include "compiler_abstract.hpp"            // For keyword OVERRIDE
#include "csl_cmp_int.hpp"                  // For csl_cmp_int
#include "can_app_message.hpp"              // for Can_app_message
#include "can_app_message_type.hpp"         // for Can_app_message_type
#include "cmsis_thread.hpp"                 // For Cmsis_thread

extern "C"
{
  #include "cfg_access_ifc.h"               // For cfg_get_descr_par_item
}

namespace comap
{

/*********************************************************************************************************************
 *  @class Shared_data
 *  @brief Template class for managing shared parameters with CAN communication.
 *
 *  This class manages a collection of shared parameters in a distributed system,
 *  providing CAN-based communication for parameter synchronization. It inherits
 *  from Can_data_handler_ifc to integrate with the CAN communication framework
 *  and handles SSV, SSRV, and SSE message types for comprehensive parameter management.
 *
 *  @tparam count Number of shared parameters to manage.
 ********************************************************************************************************************/

class Shared_data
{
  public:

    /*******************************************************************************************************************
     *  @enum message_type_t
     *  @brief Message type enumeration for CAN data identification.
     *
     *  Defines the different types of messages used in the shared data system.
     *******************************************************************************************************************/
    enum e_message_type
    {
      SSV_MESSAGE,    ///< Shared Setpoint Value message.
      SSE_MESSAGE,    ///< Shared Setpoint Error message.
      SSRV_MESSAGE,   ///< Shared Setpoint Request Value message.
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

    /*******************************************************************************************************************
     *  @brief Constructor for Shared_data.
     *
     *  @param[in] ptr_can_app_ic Pointer to CAN application interface.
     *  @param[in] addr_name Controller address name.
     *
     *  Initializes the shared data system with CAN communication capabilities.
     *******************************************************************************************************************/
    Shared_data();

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
    bool add_ssrv_message(const uint16_t param_num, const uint8_t *new_param_val);

    /*******************************************************************************************************************
     *  @brief Handle incoming CAN data messages.
     *
     *  @param[in] can_data CAN data structure containing message information.
     *
     *  @return true if message was handled successfully.
     *
     *  Processes incoming CAN messages and routes them to the appropriate handlers.
     *******************************************************************************************************************/
    bool get_message(Can_app_message& can_app_message);

    /*******************************************************************************************************************
     *  @brief Initialize the shared data system.
     *
     *  Performs initialization required by the Can_data_handler_ifc interface.
     *******************************************************************************************************************/
    void initialize(cmsis::Cmsis_thread *thread_ptr, const uint8_t cu_can_address, const uint32_t event_mask);

    /*******************************************************************************************************************
     *  @brief Get data length for specified data index.
     *
     *  @param[in] data_idx Data index to query.
     *
     *  @return Data length in bytes, or NO_DATA if index is invalid.
     *
     *  Retrieves the length of data associated with a specific message type.
     *******************************************************************************************************************/
    bool process_message(const Can_app_message& can_app_message);

    /***************************************************************************************************************//**
     * @brief Do processing of periodic tasks. It should be called periodically (40 ms).
     *
     * @param active_controllers ... bit mask of active controllers
     *
     * @thread_safety_NO
     * @call_from_ISR_NO
     ******************************************************************************************************************/
    void service();

    /*******************************************************************************************************************
     *  @brief Check if synchronization parameters are synced.
     *
     *  @return true if synchronization parameters are synced, false otherwise.
     *******************************************************************************************************************/
    bool sync_param_are_synced();

    /*******************************************************************************************************************
     *  @brief Periodic processing counter.
     *
     *  Performs periodic tasks for parameter synchronization and message processing.
     *  Should be called regularly to maintain system operation.
     *******************************************************************************************************************/
    inline void period_counter() { ++tick; }

  private:

    static const uint16_t SSV_ALL_PERIOD = 500;                   ///< SSV all message period.
    static const uint8_t  COUNT = NUM_SYNC_PARAM;                 ///< Number of shared parameters.
    static const uint8_t  SSV_PERIOD = 5;                         ///< SSV message period.
    static const uint8_t  SSRV_PERIOD = 2;                        ///< SSRV message period.
    static const uint8_t  SSE_PERIOD = 5;                         ///< SSE message period.
    static const uint8_t  SSRV_ATTEMPTS = 3;                      ///< SSRV attempt count.
    static const uint8_t  SSRV_WAIT_TICKS = 25;                   ///< SSRV wait time.
    static const uint8_t  TICK = 20;                              ///< Count of [ms] per tick.
    static const uint8_t  PACK_SIZE = 8;                          ///< SSRV message pack size.
    static const uint8_t  MIN_ACT_TICK = TICK * SSV_PERIOD;       ///< Minimum active tick count for SSV.
    static const uint8_t  QUEUE_SIZE = COUNT;                     ///< Queue size calculation.
    static const uint8_t  SHARED_PARM_MAX_DATA_LEN = 4;           ///< Max parameter data length.
    static const uint8_t  MAX_DATA_LEN = 59;                      ///< Max CAN message size.
    static const uint8_t  MAX_FLAGS_COUNT = 16;                   ///< Max count of message type flags.
    static const uint8_t  MAX_MESSAGE_SIZE = 8;                   ///< Max size of individual message.

    static const uint8_t  SSV_MSG_FLAG = 1 << SSV_MESSAGE;        ///< SSV message flag.
    static const uint8_t  SSE_MSG_FLAG = 1 << SSE_MESSAGE;        ///< SSE message flag.
    static const uint16_t SSRV_MSG_FLAGS[PACK_SIZE];              ///< SSRV message flags.

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

    struct ssv_message_t
    {
      uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];                ///< Parameter value to set.
      uint16_t  iterator;                                           ///< Sequence iterator for message ordering.
      uint16_t  param_num;                                          ///< Parameter number identifier.

      // Constructor to initialize ssv_message_t from raw data
      ssv_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
    };

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
      uint8_t   param_val[SHARED_PARM_MAX_DATA_LEN];                ///< Requested parameter value
      uint16_t  param_num;                                          ///< Parameter number identifier

      // Constructor to initialize ssrv_message_t from raw data
      ssrv_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
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

      // Constructor to initialize sse_message_t from raw data
      sse_message_t(const uint8_t *ptr_data = nullptr, const uint16_t data_len = 0);
    };
    #include "pack_struct_end.h"

    /*******************************************************************************************************************
     *  @struct can_data_t
     *  @brief CAN data structure for message transmission.
     *
     *  Structure that holds message data for CAN communication including
     *  message type, data payload, and indexing information.
     *******************************************************************************************************************/
    struct can_data_t
    {
      uint8_t  data[MAX_DATA_LEN];              ///< Message data payload.
      uint8_t  messages_size[MAX_FLAGS_COUNT];  ///< Length of data payload.
      uint8_t  data_len;                        ///< Length of data payload.
      uint16_t idx_can;                         ///< CAN-specific iterator index.
      uint16_t offset;                          ///< Data offset for multi-message handling.
      uint16_t message_type_flags;              ///< Message type identifier.

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

    Shared_param                      shared_params[COUNT]; ///< Array of shared parameters.
    Shared_buffer                     shared_buffer;        ///< Shared buffer for parameter data.
    FSQueue<sse_service_t, PACK_SIZE> sse_queue;            ///< SSE service queue.
    FSQueue<uint8_t, COUNT>           ssrv_queue;           ///< SSRV service queue.
    uint32_t                          shared_data_event;    ///< Event mask for shared data processing.
    cmsis::Cmsis_thread*              ptr_thread;           ///< Pointer to the thread handling CAN communication.
    uint16_t                          idx_ssv;              ///< SSV index counter.
    uint16_t                          idx_ssv_new;          ///< For immediately send new comm obj value.
    uint16_t                          tick;                 ///< Periodic counter.
    uint16_t                          cu_id;                ///< Controller can address.
    uint8_t                           state;                ///< State of class.

    /*******************************************************************************************************************
     *  @brief Check SSRV end counters for timeout handling.
     *
     *  @return true if it time to send SSV message.
     *  @return false if not.
     *******************************************************************************************************************/
    bool  check_counter_ssv() const;

    /*******************************************************************************************************************
     *  @brief Check SSRV end counters for timeout handling.
     *
     *  @return Number of parameters with expired counters.
     *******************************************************************************************************************/
    uint16_t check_ssrv_wait_counter();

    /*******************************************************************************************************************
     *  @brief Check if SSRV new value is available.
     *
     *  @return true if new value is available, false otherwise.
     *******************************************************************************************************************/
    bool  check_ssrv_new_value();

    /*******************************************************************************************************************
     *  @brief Get total length of all communication object parameters.
     *
     *  @param[in] comm_obj_idx Optional index of specific communication object.
     *
     *  @return Total length of all communication object parameters until comm_obj_idx.
     *******************************************************************************************************************/
    uint16_t get_all_comm_obj_len(const uint16_t comm_obj_idx = COUNT) const;

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
    bool get_ssv_msg_data(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Set SSRV message to CAN data structure.
     *
     *  @param[out] can_data Reference to store CAN data.
     *
     *  @return true if message was set successfully.
     *******************************************************************************************************************/
    bool get_ssrv_msg_data(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Set SSE message to CAN data structure.
     *
     *  @param[out] can_data Reference to store CAN data.
     *
     *  @return true if message was set successfully.
     *******************************************************************************************************************/
    bool get_sse_msg_data(can_data_t &can_data);

    /*******************************************************************************************************************
     *  @brief Get parameter index by parameter number.
     *
     *  @param[in] p_num Parameter number.
     *
     *  @return Parameter index.
     *******************************************************************************************************************/
    uint8_t get_sync_param_list_idx(const uint16_t p_num) const;

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
     *  @brief Manage SSRV message timing and retries.
     *
     *  @param[in] ssrv_idx Index of the SSRV message being managed.
     *******************************************************************************************************************/
    void ssrv_time_management(const uint16_t ssrv_idx);

    /*******************************************************************************************************************
     *  @brief Service function for SSV messages.
     *******************************************************************************************************************/
    void service_ssv();

    /*******************************************************************************************************************
     *  @brief Service function for SSRV messages.
     *******************************************************************************************************************/
    void service_ssrv();

    /*******************************************************************************************************************
     *  @brief Service function for SSE messages.
     *******************************************************************************************************************/
    void service_sse();

    /*******************************************************************************************************************
     *  @brief Service function for shared parameters.
     *******************************************************************************************************************/
    void service_shared_param();

    /*******************************************************************************************************************
     *  @brief Service function for WRN state messages.
     *******************************************************************************************************************/
    void service_wrn_state();

    /*******************************************************************************************************************
     *  @brief Set message request.
     *******************************************************************************************************************/
    void set_msg_request();

    /*******************************************************************************************************************
     *  @brief Service function for synchronization state.
     *******************************************************************************************************************/
    void service_sync_state();

    /*******************************************************************************************************************
     *  @brief Write SSV data into message.
     *
     *  @param[in] idx Parameter index.
     *
     *  @return true if data was written successfully, false otherwise.
     *******************************************************************************************************************/
    bool  write_ssv_data(const uint16_t idx, ssv_message_t &message);

    /*******************************************************************************************************************
     *  @brief Get communication object number for a given synchronization parameter index in sync_param_list array.
     *
     *  @param[in] sync_param_idx Index in the synchronization parameter list.
     *
     *  @return Communication object number corresponding to the synchronization parameter.
     *******************************************************************************************************************/
    inline uint16_t get_param_co_num(const uint16_t sync_param_idx) const
    {
      rt_assert((sync_param_idx < COUNT), "Bad index of sync_param_list");
      return sync_param_list[sync_param_idx];
    }

    /*******************************************************************************************************************
     *  @brief Check if configuration is valid.
     *
     *  @return true if configuration is valid, false otherwise.
     *******************************************************************************************************************/
    inline bool is_cfg_valid() const
    {
      return Bit::test(state, IS_CFG_VALID);
    }

 };

} // namespace comap

#endif
