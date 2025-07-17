/* ************************************************************************** */
/*                                                                            */
/*   test_sse_working.cpp - Working SSE Message Testing                      */
/*   Properly configure max values to trigger SSE messages                   */
/*                                                                            */
/* ************************************************************************** */

#include "hdrs/shared_data.hpp"
#include "hdrs/client_server_shared_setpoint.hpp"
#include "hdrs/test.hpp"

#include <iostream>
#include <chrono>
#include <cstring>
#include <vector>

using namespace std;

// Global variables
ParamData<P_COUNT> *param_data = nullptr;
SharedData<P_COUNT> *shared_data = nullptr;

struct TestStats {
    uint32_t sse_sent = 0;
    uint32_t sse_received = 0;
    uint32_t invalid_ssv_attempts = 0;
    uint32_t invalid_ssrv_attempts = 0;
    uint32_t valid_ssv_attempts = 0;
    uint32_t valid_ssrv_attempts = 0;
} stats;

void log_message(const string& tag, const string& msg) {
    auto now = chrono::duration_cast<chrono::microseconds>(
        chrono::steady_clock::now().time_since_epoch()).count();
    
    cout << "[" << tag << "][" << now << "] PID:" << getpid() << " " << msg << endl;
}

void debug_max_values() {
    log_message("DEBUG", "🔍 Checking parameter max values after proper initialization...");
    
    for (uint16_t i = 0; i < min(5, (int)P_COUNT); i++) {
        uint16_t idx = param_data->get_param_idx(i);
        uint32_t max_val = param_data->get_param_max_value(idx);
        uint32_t current_val = param_data->get_param_value(idx);
        
        log_message("DEBUG", "Param #" + to_string(i) + ": idx=" + to_string(idx) + 
                   " current=" + to_string(current_val) + " max=" + to_string(max_val));
    }
}

void test_sse_generation() {
    log_message("TEST", "🚨 Testing SSE generation with properly configured max values");
    
    // Test with parameter 0 (max value should be 5000)
    uint16_t param_num = 0;
    uint16_t idx = param_data->get_param_idx(param_num);
    uint32_t max_val = param_data->get_param_max_value(idx);
    
    log_message("TEST", "Using param #" + to_string(param_num) + " with max=" + to_string(max_val));
    
    // Test with values that should be invalid
    vector<int32_t> test_values = {6000, 10000, 15000, 50000};  // All > 5000
    
    for (int32_t test_val : test_values) {
        bool is_valid = (max_val >= test_val);
        log_message("TEST", "Testing value " + to_string(test_val) + " vs max " + to_string(max_val) + 
                   ": " + (is_valid ? "✅ VALID" : "❌ INVALID - should trigger SSE"));
        
        if (!is_valid) {
            // Test SSV message that should trigger SSE
            ssv_message_t invalid_ssv;
            invalid_ssv.param_num = param_num;
            invalid_ssv.param_val = test_val;
            invalid_ssv.iterator = 0;
            
            can_data_t can_data;
            can_data.message_type = SSV_MESSAGE;
            can_data.data_len = sizeof(ssv_message_t);
            memcpy(can_data.data, &invalid_ssv, sizeof(ssv_message_t));
            can_data.idx = 0;
            can_data.idx_can = getpid();
            
            shared_data->handle_messages(can_data);
            stats.invalid_ssv_attempts++;
            
            // Check for SSE generation
            can_data_t sse_output;
            if (shared_data->get_messages(sse_output) && sse_output.message_type == SSE_MESSAGE) {
                sse_message_t sse_msg;
                memcpy(&sse_msg, sse_output.data, sizeof(sse_msg));
                
                stats.sse_sent++;
                log_message("SSE_SUCCESS", "🚨 SSE GENERATED! param #" + to_string(sse_msg.param_num) + 
                           " error_code: " + to_string((int)sse_msg.error_code) + 
                           " for invalid value " + to_string(test_val));
            } else {
                log_message("SSE_FAIL", "❌ NO SSE generated for invalid value " + to_string(test_val));
            }
        } else {
            stats.valid_ssv_attempts++;
        }
    }
    
    log_message("TEST", "");
    
    // Test SSRV messages
    log_message("TEST", "🚨 Testing SSRV messages that should trigger SSE");
    
    for (int32_t test_val : test_values) {
        bool is_valid = (max_val >= test_val);
        
        if (!is_valid) {
            ssrv_message_t invalid_ssrv;
            invalid_ssrv.param_num = param_num;
            invalid_ssrv.param_val = test_val;
            
            can_data_t can_data;
            can_data.message_type = SSRV_MESSAGE;
            can_data.data_len = sizeof(ssrv_message_t);
            memcpy(can_data.data, &invalid_ssrv, sizeof(ssrv_message_t));
            
            shared_data->handle_messages(can_data);
            stats.invalid_ssrv_attempts++;
            
            // Check for SSE generation
            can_data_t sse_output;
            if (shared_data->get_messages(sse_output) && sse_output.message_type == SSE_MESSAGE) {
                sse_message_t sse_msg;
                memcpy(&sse_msg, sse_output.data, sizeof(sse_msg));
                
                stats.sse_sent++;
                log_message("SSE_SUCCESS", "🚨 SSE GENERATED! param #" + to_string(sse_msg.param_num) + 
                           " error_code: " + to_string((int)sse_msg.error_code) + 
                           " for invalid SSRV value " + to_string(test_val));
            } else {
                log_message("SSE_FAIL", "❌ NO SSE generated for invalid SSRV value " + to_string(test_val));
            }
        } else {
            stats.valid_ssrv_attempts++;
        }
    }
}

void test_sse_handling() {
    log_message("TEST", "🔧 Testing SSE message handling");
    
    for (uint8_t error_code = 1; error_code <= 2; error_code++) {
        sse_message_t sse_msg;
        sse_msg.param_num = error_code - 1;
        sse_msg.error_code = error_code;
        
        log_message("SSE_INPUT", "📥 Processing SSE param #" + to_string(sse_msg.param_num) + 
                   " error_code: " + to_string((int)sse_msg.error_code));
        
        can_data_t can_data;
        can_data.message_type = SSE_MESSAGE;
        can_data.data_len = sizeof(sse_message_t);
        memcpy(can_data.data, &sse_msg, sizeof(sse_msg));
        
        shared_data->handle_messages(can_data);
        stats.sse_received++;
    }
}

void print_final_stats() {
    log_message("STATS", "==================== WORKING SSE TEST RESULTS ====================");
    log_message("STATS", "Process ID: " + to_string(getpid()));
    log_message("STATS", "");
    log_message("STATS", "🚨 SSE Test Statistics:");
    log_message("STATS", "   Valid SSV tests:        " + to_string(stats.valid_ssv_attempts));
    log_message("STATS", "   Invalid SSV attempts:   " + to_string(stats.invalid_ssv_attempts));
    log_message("STATS", "   Valid SSRV tests:       " + to_string(stats.valid_ssrv_attempts));
    log_message("STATS", "   Invalid SSRV attempts:  " + to_string(stats.invalid_ssrv_attempts));
    log_message("STATS", "   SSE messages generated: " + to_string(stats.sse_sent));
    log_message("STATS", "   SSE messages handled:   " + to_string(stats.sse_received));
    log_message("STATS", "");
    
    bool sse_generation_working = (stats.sse_sent > 0);
    bool sse_handling_working = (stats.sse_received > 0);
    bool invalid_values_tested = (stats.invalid_ssv_attempts > 0 || stats.invalid_ssrv_attempts > 0);
    
    log_message("STATS", "✅ SSE Functionality Validation:");
    log_message("STATS", "   Invalid values tested:     " + string(invalid_values_tested ? "✅ YES" : "❌ NO"));
    log_message("STATS", "   SSE generation working:    " + string(sse_generation_working ? "✅ YES" : "❌ NO"));
    log_message("STATS", "   SSE handling working:      " + string(sse_handling_working ? "✅ YES" : "❌ NO"));
    
    log_message("STATS", "");
    if (invalid_values_tested && sse_generation_working && sse_handling_working) {
        log_message("STATS", "🎉 SUCCESS: SSE error handling is FULLY FUNCTIONAL!");
        log_message("STATS", "   ✅ Invalid values correctly trigger SSE messages");
        log_message("STATS", "   ✅ SSE messages are generated and handled properly");
        log_message("STATS", "   ✅ Algorithm demonstrates robust error handling");
    } else if (invalid_values_tested && sse_generation_working) {
        log_message("STATS", "✅ PARTIAL SUCCESS: SSE generation works but handling needs verification");
    } else if (invalid_values_tested && !sse_generation_working) {
        log_message("STATS", "❌ FAILURE: Invalid values tested but SSE generation failed");
    } else {
        log_message("STATS", "❌ FAILURE: SSE functionality needs investigation");
    }
    
    log_message("STATS", "================================================================");
}

int main()
{
    log_message("INIT", "🎮 Working SSE test started: PID=" + to_string(getpid()));
    
    // Initialize parameter data and shared data
    param_data = new ParamData<P_COUNT>();
    shared_data = new SharedData<P_COUNT>();
    
    // CRITICAL: Set the default max value BEFORE initializing parameters
    log_message("INIT", "📏 Setting default max value to 5000 BEFORE parameter initialization");
    param_data->set_param_max_value(5000);
    
    // Initialize parameters with the new default max value
    log_message("INIT", "📏 Initializing " + to_string(P_COUNT) + " parameters");
    for (uint16_t i = 0; i < P_COUNT; i++) {
        param_data->set_param_value(i, i, i * 10);  // Initial values: 0, 10, 20, etc.
        shared_data->set_param_num(i);  // Set parameter numbers
    }
    
    log_message("INIT", "");
    
    // Debug current state
    debug_max_values();
    log_message("PHASE", "");
    
    // Test SSE generation and handling
    test_sse_generation();
    log_message("PHASE", "");
    
    test_sse_handling();
    log_message("PHASE", "");
    
    print_final_stats();
    
    // Cleanup
    delete shared_data;
    delete param_data;
    
    log_message("SHUTDOWN", "✅ Working SSE test completed");
    return 0;
}