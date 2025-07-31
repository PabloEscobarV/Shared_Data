/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shared_param.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 21:39:08 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/31 15:25:20 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_PARAM_HPP
#define SHARED_PARAM_HPP

#include "p_iterator.hpp"

#include <cstdint>

#include <mutex>

/*********************************************************************************************************************
 *  @class SharedParam
 *  @brief Individual parameter management with synchronization and error handling.
 *
 *  This class manages a single parameter in a distributed system, providing synchronized
 *  access, value validation, and conflict resolution. It uses iterator-based message
 *  ordering to ensure consistent parameter states across multiple nodes. The class
 *  handles different message types (SSV, SSRV, SSE) and maintains error state information
 *  using bit manipulation for efficient storage.
 ********************************************************************************************************************/

class SharedParam
{
  public:
    /*******************************************************************************************************************
     *  @enum e_errorcode
     *  @brief Error code enumeration for parameter operations.
     *
     *  Defines various error conditions that can occur during parameter operations.
     *******************************************************************************************************************/
    enum e_errorcode
    {
      NO_ERROR,                    ///< No error occurred
      OUT_OF_RANGE_SSV,           ///< SSV parameter value is out of acceptable range
      OUT_OF_RANGE_SSRV,          ///< SSRV parameter value is out of acceptable range
      NEW_VAL_REQ_NOT_ALLOWED,    ///< New value request is not permitted in current state
      SET_SSRV_COUNTER,           ///< Error counter for SSRV operations
    };

    /*******************************************************************************************************************
     *  @brief Constructor for SharedParam.
     *
     *  @param[in] p_idx Parameter number to manage (default: 0).
     *******************************************************************************************************************/
    SharedParam(const uint16_t p_idx = 0);

    /*******************************************************************************************************************
     *  @brief Initialize the parameter with a specific parameter number.
     *
     *  @param[in] p_idx Parameter number to set.
     *******************************************************************************************************************/
    void init(const uint16_t p_idx);

    /*******************************************************************************************************************
     *  @brief Accept the currently pending new parameter value.
     *
     *  @return true if value was accepted, false if no pending value exists.
     *
     *  Applies the pending parameter value and clears the new value request flag.
     *  Used after successful validation and synchronization.
     *******************************************************************************************************************/
    bool accept_new_value();

    /*******************************************************************************************************************
     *  @brief Add a new parameter value with SSRV attempt counter.
     *
     *  @param[in] new_param_val The new parameter value to set.
     *  @param[in] ssrv_atmp_counter SSRV attempt counter for validation.
     *  @return true if value was added successfully, false on validation failure.
     *
     *  Stores a new parameter value with associated attempt counter for later validation.
     *  Performs range checking and state validation before acceptance.
     *******************************************************************************************************************/
    bool add_new_param_value(uint8_t ssrv_atmp_counter, uint8_t *new_param_val, uint16_t size = SHARED_PARM_MAX_DATA_LEN);

    /*******************************************************************************************************************
     *  @brief Set the SSRV end counter value.
     *
     *  @param[in] counter The counter value to set.
     *  Sets the end counter for SSRV operations, used for synchronization validation.
     *******************************************************************************************************************/
    void set_ssrv_end_counter(uint8_t counter);

    /*******************************************************************************************************************
     *  @brief Reset the SSRV end counter to initial state.
     *
     *  Clears the SSRV end counter, typically called when starting new operations.
     *******************************************************************************************************************/
    void reset_ssrv_end_counter();

    /*******************************************************************************************************************
     *  @brief Get the current SSRV end counter value.
     *
     *  @param[out] counter Reference to store the counter value.
     *  @return true if counter is valid, false if not set.
     *******************************************************************************************************************/
    bool get_ssrv_end_counter(uint8_t& counter) const;

    /*******************************************************************************************************************
     *  @brief Check if new value requests are currently allowed.
     *
     *  @return true if new values can be accepted, false otherwise.
     *
     *  Determines if the parameter is in a state where new value requests
     *  can be processed based on current synchronization status.
     *******************************************************************************************************************/
    bool is_new_value_allowed() const;

    /*******************************************************************************************************************
     *  @brief Get SSV (Set Shared Value) message for transmission.
     *
     *  @param[out] message Reference to populate with SSV message data.
     *  @return true if message was populated, false if no message available.
     *******************************************************************************************************************/
    uint16_t get_iterator() const;

    /*******************************************************************************************************************
     *  @brief Get SSRV (Set Shared Request Value) message for transmission.
     *
     *  @param[out] message Reference to populate with SSRV message data.
     *  @return true if message was populated, false if no message available.
     *******************************************************************************************************************/
    bool get_new_value(uint8_t *data, uint16_t size = SHARED_PARM_MAX_DATA_LEN) const;

    /*******************************************************************************************************************
     *  @brief Get SSE (Set Shared Error) message for transmission.
     *
     *  @param[out] message Reference to populate with SSE message data.
     *  @return true if message was populated, false if no error to report.
     *******************************************************************************************************************/
    uint8_t get_error_code() const;

    /*******************************************************************************************************************
     *  @brief Handle incoming SSV message.
     *
     *  @param[in] message The SSV message to process.
     *  @param[in] idx Current iterator index.
     *  @param[in] idx_can CAN-specific iterator index.
     *  @return true if message was processed successfully, false on error.
     *
     *  Processes an incoming SSV message, validates the parameter value,
     *  and updates internal state based on iterator ordering logic.
     *******************************************************************************************************************/
    bool handle_p_synchro(const uint8_t *param_val,
                          const uint16_t iter_synchro,
                          const uint16_t idx,
                          const uint16_t idx_can);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSRV message.
     *
     *  @param[in] message The SSRV message to process.
     *  @return true if message was processed successfully, false on error.
     *
     *  Processes an incoming SSRV message, validates the request parameters,
     *  and manages the coordination protocol for value changes.
     *******************************************************************************************************************/
    bool handle_p_val_change_req(const uint8_t *new_p_val);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSE message.
     *
     *  @param[in] message The SSE message to process.
     *  @return true if message was processed successfully, false on error.
     *
     *  Processes an incoming error message and updates local error state
     *  to maintain consistency with the distributed system error status.
     *******************************************************************************************************************/
    bool handle_error_code(const uint8_t error_code);

    /*******************************************************************************************************************
     *  @brief Get the parameter number managed by this instance.
     *
     *  @return The parameter number.
     *******************************************************************************************************************/
    uint16_t get_param_num() const;

    /*******************************************************************************************************************
     *  @brief Get the current parameter value.
     *
     *  @return The current parameter value.
     *******************************************************************************************************************/
    bool get_param_value(uint8_t* dest) const;

  private:
    static const uint8_t SSRV_INCR_VALUE = 3;  ///< SSRV increment value constant

    P_Iterator *iterator;                                 ///< Iterator for message ordering
    uint16_t   p_idx;                                     ///< Parameter index identifier
    uint8_t    ssrv_counter;                              ///< SSRV operation counter
    uint8_t    err_code;                                  ///< Current error code state
    uint8_t    new_param_value[SHARED_PARM_MAX_DATA_LEN]; ///< Pending new parameter value

    /*******************************************************************************************************************
     *  @brief Check if parameter value update is requested.
     *
     *  @param[in] message The SSV message to evaluate.
     *  @param[in] idx Current iterator index.
     *  @param[in] idx_can CAN-specific iterator index.
     *  @return true if update is requested, false otherwise.
     *******************************************************************************************************************/
    bool is_req_update_param_value(const uint16_t iter_synchro, const uint16_t idx, const uint16_t idx_can);

    /*******************************************************************************************************************
     *  @brief Get the maximum allowed value for the parameter.
     *
     *  @return The maximum parameter value.
     *******************************************************************************************************************/
    bool is_param_value_ok(const uint8_t *p_value, uint16_t size = SHARED_PARM_MAX_DATA_LEN) const;

    /*******************************************************************************************************************
     *  @brief Write the parameter value into memory.
     *
     *  @param[in] p_value The new parameter value to set.
     *******************************************************************************************************************/
    void write_param_value(const uint8_t *p_value, uint16_t size = SHARED_PARM_MAX_DATA_LEN);

    /*******************************************************************************************************************
     *  @brief Set the parameter value.
     *
     *  @param[in] p_value The new parameter value to set.
     *******************************************************************************************************************/
    void set_param_value(const co_descr_t& descr, const uint16_t& comm_obj, uint32_t& received_value);

    /*******************************************************************************************************************
     *  @brief Update the internal iterator for message ordering.
     *******************************************************************************************************************/
    void update_iterator();
};

#endif // SHARED_PARAM_HPP