/***********************************************************************************************************************
 *  @file shared_param.hpp
 *  @brief Shared parameter management for distributed parameter synchronization.
 *
 *  @par Copyright (c) 2024 ComAp a.s  All rights reserved.
 **********************************************************************************************************************/

#ifndef SHARED_PARAM_HPP
#define SHARED_PARAM_HPP

#include "../p_iterator/p_iterator.hpp"       // P_Iterator – iterator with wrap-around logic
#include "../shared_buffer/shared_buffer.hpp" // For Shared_buffer
#include "compiler_abstract.hpp"              // For keyword OVERRIDE
#include "csl_bit.hpp"                        // For Bit manipulation
#include "csl_cmp_int.hpp"                    // For csl_cmp_int

extern "C"
{
  #include "app_comm_obj_descr.h"
  #include "cfg_il3f_types.h"           // cfg_il3f_types
  #include "stdfloat.h"                 // For float32_t
}

// standard library includes
#include <stdint.h>                     // For fixed-width integer types

/* ===================================================================================================================
 *  Message structure definitions
 * ================================================================================================================ */

namespace comap
{

/*********************************************************************************************************************
 *  @class Shared_param
 *  @brief Individual parameter management with synchronization and error handling.
 *
 *  This class manages a single parameter in a distributed system, providing synchronized
 *  access, value validation, and conflict resolution. It uses iterator-based message
 *  ordering to ensure consistent parameter states across multiple nodes. The class
 *  handles different message types (SSV, SSRV, SSE) and maintains error state information
 *  using bit manipulation for efficient storage.
 ********************************************************************************************************************/

class Shared_param
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
      NO_ERROR,                   ///< No error occurred
      OUT_OF_RANGE_SSV,           ///< SSV parameter value is out of acceptable range
      OUT_OF_RANGE_SSV_RESET,     ///< SSV parameter value is out of acceptable range but not updated.
      OUT_OF_RANGE_SSRV,          ///< SSRV parameter value is out of acceptable range
      NEW_VAL_REQ_NOT_ALLOWED,    ///< New value request is not permitted in current state
      NEW_VAL_SEND_STATE,         ///< New value is being sent
      NEW_VAL_WAIT_STATE,         ///< New value is waiting for confirmation
      SYNCED                      ///< Parameter is synchronized
    };

    static const uint16_t SHARED_PARM_MAX_DATA_LEN = 4;     ///< Maximum shared parameter data length.

    /*******************************************************************************************************************
     *  @brief Constructor for Shared_param.
     *******************************************************************************************************************/
    Shared_param();

    /*******************************************************************************************************************
     *  @brief Accept the currently pending new parameter value.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] buffer Reference to shared buffer for parameter data.
     *
     *  @return true if value was accepted, false if no pending value exists.
     *
     *  Applies the pending parameter value and clears the new value request flag.
     *  Used after successful validation and synchronization.
     *******************************************************************************************************************/
    bool accept_new_value(const uint16_t co_num, Shared_buffer& shared_buffer);

    /*******************************************************************************************************************
     *  @brief Add a new parameter value with SSRV attempt counter.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] buffer Reference to shared buffer for parameter data.
     *  @param[in] ptr_new_param_val Pointer to the new parameter value to add.
     *
     *  @return true if value was added successfully, false on validation failure.
     *
     *  Adds a new parameter value to be sent via SSRV message, initializing the attempt counter.
     *******************************************************************************************************************/
    bool add_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, const uint8_t *ptr_new_param_val);

    /*******************************************************************************************************************
     *  @brief Get SSRV (Set Shared Request Value) message for transmission.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] buffer Reference to shared buffer for parameter data.
     *  @param[out] data Reference to populate with SSRV message data.
     *
     *  @return true if message was populated, false if no message available.
     *******************************************************************************************************************/
    bool get_new_value(const uint16_t co_num, Shared_buffer& shared_buffer, uint8_t *ptr_data) const;

    /*******************************************************************************************************************
     *  @brief Get the current parameter value.
     *
     *  @param[in] co_num The communication object identifier.
     *  @param[out] dest Pointer to the destination buffer for the parameter value.
     *
     *  @return The current parameter value.
     *******************************************************************************************************************/
    bool get_param_value(const uint16_t co_num, uint8_t* dest) const;

    /*******************************************************************************************************************
     *  @brief Handle incoming SSV message.
     *
     *  @param[in] co_num Configuration index of the parameter.
     *  @param[in] param_val Pointer to the parameter value data.
     *  @param[in] iter_synchro Iterator value from the SSV message.
     *  @param[in] id_can CAN identifier of the sender.
     *  @param[in] id_can_received CAN identifier of the receiver.
     *
     *  @return true if message was processed successfully, false on error.
     *******************************************************************************************************************/
    bool handle_ssv_value(const uint16_t co_num,
                          const uint8_t *param_val,
                          const uint16_t iter_synchro,
                          const uint16_t id_can,
                          const uint16_t id_can_received);

    /*******************************************************************************************************************
     *  @brief Handle incoming SSRV message.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] new_p_val Pointer to the new parameter value data.
     *
     *  @return true if message was processed successfully, false on error.
     *******************************************************************************************************************/
    bool handle_ssrv_value(const uint16_t co_num, const uint8_t *new_p_val);

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
     *  @brief Reset counter to initial state.
     *
     *  Clears the SSRV attempt counter, typically called when starting new operations.
     *******************************************************************************************************************/
    void reset_counter();

    /*******************************************************************************************************************
     *  @brief Set counter to a specific value and according state flags.
     *
     *  @param[in] cnt Counter value to set.
     *******************************************************************************************************************/
    void set_send_counter(const uint16_t cnt);

    /*******************************************************************************************************************
     *  @brief Set the counter to a specific value and according state flags.
     *
     *  @param[in] cnt The counter value to set.
     *******************************************************************************************************************/
    void set_wait_time_stamp(const uint16_t time_stmp);

    /*******************************************************************************************************************
     *  @brief Periodic service function to manage state transitions and timeouts.
     *
     *  Should be called regularly to handle time-based state changes and retries.
     *******************************************************************************************************************/
    void service();

    /*******************************************************************************************************************
     *  @brief Decrement the SSRV attempt counter.
     *******************************************************************************************************************/
    inline void decr_counter()
    {
      --counter;
    }

    /*******************************************************************************************************************
     *  @brief Get the current value of the SSRV attempt counter.
     *
     *  @return The current counter value.
     *******************************************************************************************************************/
    inline uint16_t get_counter() const
    {
      return counter;
    }

    /*******************************************************************************************************************
     *  @brief Get SSE (Set Shared Error) message for transmission.
     *
     *  @param[out] message Reference to populate with SSE message data.
     *  @return true if message was populated, false if no error to report.
     *******************************************************************************************************************/
    inline uint8_t get_error_code() const
    {
      return err_code;
    }

    /*******************************************************************************************************************
     *  @brief Get SSV (Set Shared Value) message for transmission.
     *
     *  @param[out] message Reference to populate with SSV message data.
     *  @return true if message was populated, false if no message available.
     *******************************************************************************************************************/
    inline uint16_t get_iterator() const
    {
      return iterator.get_iterator();
    }

    /*******************************************************************************************************************
     *  @brief Increment the SSRV attempt counter.
     *******************************************************************************************************************/
    inline void incr_counter()
    {
      ++counter;
    }

    /*******************************************************************************************************************
     *  @brief Get the new value wait state.
     *
     *  @return true if new value wait state is active, false otherwise.
     *******************************************************************************************************************/
    inline bool is_new_val_wait_state() const
    {
      return Bit::test(err_code, NEW_VAL_WAIT_STATE);
    }

    /*******************************************************************************************************************
     *  @brief Get the current new value send state.
     *
     *  @param[out] counter Reference to store the counter value.
     *  @return true if counter is valid, false if not set.
     *******************************************************************************************************************/
    inline bool is_new_val_send_state() const
    {
      return Bit::test(err_code, NEW_VAL_SEND_STATE);
    }

    /*******************************************************************************************************************
     *  @brief Get the current out-of-range error state.
     *
     *  @param[in] error_code Optional error code to check against current state.
     *
     *  @return true if out-of-range error is set, false otherwise.
     *******************************************************************************************************************/
    inline bool is_out_of_range_ssv_state(const uint8_t error_code = csl_cmp_int<uint8_t>::NOT_VALID) const
    {
      if (error_code == csl_cmp_int<uint8_t>::NOT_VALID)
      {
        return Bit::test(err_code, OUT_OF_RANGE_SSV);
      }
      return Bit::test(error_code, OUT_OF_RANGE_SSV);
    }

    /*******************************************************************************************************************
     *  @brief Get the current out-of-range SSRV error state.
     *
     *  @param[in] error_code Optional error code to check against current state.
     *
     *  @return true if out-of-range SSRV error is set, false otherwise.
     *******************************************************************************************************************/
    inline bool is_out_of_range_ssrv_state(const uint8_t error_code = csl_cmp_int<uint8_t>::NOT_VALID) const
    {
      if (error_code == csl_cmp_int<uint8_t>::NOT_VALID)
      {
        return Bit::test(err_code, OUT_OF_RANGE_SSRV);
      }
      return Bit::test(error_code, OUT_OF_RANGE_SSRV);
    }

    /*******************************************************************************************************************
     *  @brief Check if new value requests are currently allowed.
     *
     *  @return true if new values can be accepted, false otherwise.
     *
     *  Determines if the parameter is in a state where new value requests
     *  can be processed based on current synchronization status.
     *******************************************************************************************************************/
    inline bool is_new_value_allowed() const
    {
      return !Bit::test(err_code, NEW_VAL_REQ_NOT_ALLOWED);
    }

    /*******************************************************************************************************************
     *  @brief Check if parameter is synchronized.
     *
     *  @return true if parameter is synchronized, false otherwise.
     *******************************************************************************************************************/
    inline bool is_synced() const
    {
      return Bit::test(err_code, SYNCED);
    }

    /*******************************************************************************************************************
     * @brief Increment the internal iterator for message ordering.
     *
     * This method increments the internal iterator used for message sequencing.
     * It should be called after processing a message to ensure the next message
     * is correctly ordered in the sequence.
     *******************************************************************************************************************/
    inline uint16_t send_iterator()
    {
      ++iterator;
      return iterator.get_iterator();
    }

  private:

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

    static const uint8_t SSRV_INCR_VALUE = 3;               ///< SSRV increment value constant.
    static const float32_t FLOAT_PRECISION;                 ///< Precision for floating-point comparisons.

    uint16_t    counter;                                    ///< Counter for new value request.
    P_Iterator  iterator;                                   ///< Iterator for message ordering.
    uint8_t     err_code;                                   ///< Current error code state.

    /*******************************************************************************************************************
     *  @brief Check if new value requests are currently allowed.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] ptr_new_param_val Pointer to the new parameter value to validate.
     *  @param[out] descr Reference to populate with the communication object descriptor.
     *
     *  @return true if new values can be accepted, false otherwise.
     *******************************************************************************************************************/
    bool check_new_value(const uint16_t co_num, const uint8_t *ptr_new_param_val, co_descr_t& descr) const;

    /*******************************************************************************************************************
     *  @brief Compare two data values of the same type for equality.
     *
     *  @param[in] ptr_first_data Pointer to the first data value.
     *  @param[in] ptr_second_data Reference to the second data value.
     *
     *  @return true if values are equal, false otherwise.
     *******************************************************************************************************************/
    int16_t cmp_data_correct_type(const uint8_t* ptr_first_data,
                                const uint8_t* ptr_second_data,
                                const uint8_t data_type) const;

    /*******************************************************************************************************************
     *  @brief Compare two data values of the same type for ordering.
     *
     *  @param[in] a Pointer to the first data value.
     *  @param[in] b Pointer to the second data value.
     *
     *  @return -1 if a < b, 0 if a == b, 1 if a > b, -2 on error.
     *******************************************************************************************************************/
    template<typename data_t>
    int16_t cmp_data_with_type(const data_t a, const data_t b) const;

    /*******************************************************************************************************************
     *  @brief Retrieve the communication object descriptor for a given communication object number.
     *
     *  @param[in] co_num Communication object number.
     *  @param[out] descr Reference to populate with the communication object descriptor.
     *
     *  @return true if descriptor was found, false otherwise.
     *******************************************************************************************************************/
    bool get_descr(const uint16_t co_num, co_descr_t& descr) const;

    /*******************************************************************************************************************
     *  @brief Get setpoint value from configuration.
     *
     *  @param setpoint_idx ... index of the setpoint in configuration table
     *  @param setpoint_description ... setpoint description from configuration table
     *
     *  @return setpoint value
     ******************************************************************************************************************/
    setpoint_limits_t get_setpoint_limits(const cfg_il3f_descr_par_t& setpoint_description) const;

    /*******************************************************************************************************************
     *  @brief Check if new parameter value differs from current value.
     *
     *  @param[in] descr Reference to the communication object descriptor.
     *  @param[in] ptr_new_param_value Pointer to the new parameter value to compare.
     *
     *  @return true if values differ, false if they are the same.
     *******************************************************************************************************************/
    bool is_data_new(const co_descr_t& descr, const uint8_t *ptr_new_param_value) const;

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
     *  @brief Check if parameter value is out of range.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] data Pointer to the parameter value data.
     *  @param[in] size Size of the parameter value in bytes.
     *
     *  @return true if value is out of range, false otherwise.
     *******************************************************************************************************************/
    bool is_param_val_in_range(const uint16_t co_num, const uint8_t *ptr_new_data) const;

    /*******************************************************************************************************************
     *  @brief Read the parameter value from memory.
     *
     *  @param[in] param_cfg_idx Configuration index of the parameter.
     *
     *  @return The parameter value read from memory.
     *******************************************************************************************************************/
    uint32_t read_param_value(const uint16_t param_cfg_idx) const;

    /*******************************************************************************************************************
     *  @brief Compare two data values of the same type for ordering.
     *
     *  @param[in] ptr_first_data Pointer to the first data value.
     *  @param[in] ptr_second_data Pointer to the second data value.
     *
     *  @return -1 if first < second, 0 if first == second, 1 if first > second, -2 on error.
     *******************************************************************************************************************/
    template <typename data_t>
    int16_t set_cmp_data(const uint8_t* ptr_first_data, const uint8_t* ptr_second_data) const;

    /*******************************************************************************************************************
     *  @brief Set the parameter value.
     *
     *  @param[in] p_value The new parameter value to set.
     *******************************************************************************************************************/
    bool set_param_value(const uint16_t param_cfg_idx,
                        const co_descr_t& descr,
                        const uint8_t *ptr_param_value = nullptr) const;

    /*******************************************************************************************************************
     *  @brief Write the parameter value into memory.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[in] ptr_new_param_value Pointer to the new parameter value to write
     *
     *  @return true if value was written successfully, false otherwise.
     *******************************************************************************************************************/
    /* see header file */
    bool write_param_value(const uint16_t co_num, const uint8_t *ptr_new_param_value) const;

    /*******************************************************************************************************************
     *  @brief Write the parameter value into memory.
     *
     *  @param[in] co_num Communication object number of the parameter.
     *  @param[out] shared_buffer Reference to shared buffer for parameter data.
     *
     *  @return true if value was written successfully, false otherwise.
     *******************************************************************************************************************/
    bool write_param_value(const uint16_t co_num, Shared_buffer& shared_buffer) const;

    /*******************************************************************************************************************
     *  @brief Check if out-of-range SSV reset state is active.
     *
     *  @return true if out-of-range SSV reset state is active, false otherwise.
     *******************************************************************************************************************/
    inline bool is_out_of_range_ssv_reset_state() const
    {
      return Bit::test(err_code, OUT_OF_RANGE_SSV_RESET);
    }

    /*******************************************************************************************************************
     *  @brief Update the internal iterator for message ordering.
     *******************************************************************************************************************/
    inline void update_iterator()
    {
      iterator += SSRV_INCR_VALUE + 1;
    }

    /*******************************************************************************************************************
     *  @brief Set the new value request not allowed state.
     *******************************************************************************************************************/
    inline void set_new_value_not_allowed_state()
    {
      Bit::set(err_code, NEW_VAL_REQ_NOT_ALLOWED);
    }

    /*******************************************************************************************************************
     *  @brief Reset the new value request not allowed state.
     *******************************************************************************************************************/
    inline void reset_new_value_not_allowed_state()
    {
      Bit::clear(err_code, NEW_VAL_REQ_NOT_ALLOWED);
    }

    /*******************************************************************************************************************
     *  @brief Set the out-of-range error state.
     *******************************************************************************************************************/
    inline void set_out_of_range_ssv_state()
    {
      Bit::set(err_code, OUT_OF_RANGE_SSV);
    }

    /*******************************************************************************************************************
     *  @brief Set the out-of-range reset error state.
     *******************************************************************************************************************/
    inline void set_out_of_range_ssv_reset_state()
    {
      Bit::set(err_code, OUT_OF_RANGE_SSV_RESET);
    }

    /*******************************************************************************************************************
     *  @brief Reset the out-of-range error state.
     *******************************************************************************************************************/
    inline void reset_out_of_range_ssv_state()
    {
      Bit::clear(err_code, OUT_OF_RANGE_SSV);
    }

    /*******************************************************************************************************************
     *  @brief Reset the out-of-range reset error state.
     *******************************************************************************************************************/
    inline void reset_out_of_range_ssv_reset_state()
    {
      Bit::clear(err_code, OUT_OF_RANGE_SSV_RESET);
    }

    /*******************************************************************************************************************
     *  @brief Set the out-of-range SSRV error state.
     *******************************************************************************************************************/
    inline void set_out_of_range_ssrv_state()
    {
      Bit::set(err_code, OUT_OF_RANGE_SSRV);
    }

    /*******************************************************************************************************************
     *  @brief Reset the out-of-range SSRV error state.
     *******************************************************************************************************************/
    inline void reset_out_of_range_ssrv_state()
    {
      Bit::clear(err_code, OUT_OF_RANGE_SSRV);
    }

    /*******************************************************************************************************************
     *  @brief Set the SSRV end counter value.
     *
     *  Sets the end counter for SSRV operations, used for synchronization validation.
     *******************************************************************************************************************/
    inline void set_new_val_send_state()
    {
      Bit::set(err_code, NEW_VAL_SEND_STATE);
    }

    /*******************************************************************************************************************
     *  @brief Reset the SSRV end counter to initial state.
     *
     *  Clears the SSRV end counter, typically called when starting new operations.
     *******************************************************************************************************************/
    inline void reset_new_val_send_state()
    {
      Bit::clear(err_code, NEW_VAL_SEND_STATE);
    }

    /*******************************************************************************************************************
     *  @brief Set the new value wait state.
     *
     *  Sets the wait state for new value requests, used for synchronization validation.
     *******************************************************************************************************************/
    inline void set_new_val_wait_state()
    {
      Bit::set(err_code, NEW_VAL_WAIT_STATE);
    }

    /*******************************************************************************************************************
     *  @brief Reset the new value wait state.
     *
     *  Clears the wait state for new value requests, typically called when starting new operations.
     *******************************************************************************************************************/
    inline void reset_new_val_wait_state()
    {
      Bit::clear(err_code, NEW_VAL_WAIT_STATE);
    }
};

} // namespace comap


#endif // SHARED_PARAM_HPP
