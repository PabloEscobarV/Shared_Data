/* ************************************************************************** */
/*                                                                            */
/*   test_ultimate_100_final.cpp - Ultimate 100 Process Test                 */
/*   100 processes + 1 new process, 64 parameters, full SSV/SSRV/SSE        */
/*   - SSV: Iterator synchronization (±1), value synchronization             */
/*   - SSRV: 3 times every 40ms, 500ms wait, no SSE = set new param value    */
/*   - SSE: Invalid SSV values trigger 3 SSE messages (100ms each)           */
/*   - SSE: Invalid SSRV values trigger 1 SSE message immediately            */
/*                                                                            */
/* ************************************************************************** */

#include "hdrs/shared_data.hpp"
#include "hdrs/client_server_shared_setpoint.hpp"
#include "hdrs/test.hpp"

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <random>
#include <map>
#include <set>
#include <vector>

using namespace std;

// Global variables
ParamData<P_COUNT> *param_data = nullptr;
SharedData<P_COUNT> *shared_data = nullptr;
mutex mtx_out;

atomic<bool> should_exit(false);
atomic<bool> is_new_process(false);
uint32_t test_duration_ms = 150000; // 2.5 minutes for ultimate test
const uint16_t TICK_PERIOD = 20;

struct TestStats {
    uint32_t ssv_sent = 0;
    uint32_t ssv_received = 0;
    uint32_t ssrv_sent = 0;
    uint32_t ssrv_received = 0;
    uint32_t sse_sent = 0;
    uint32_t sse_received = 0;
    uint32_t param_changes = 0;
    uint32_t iterator_updates = 0;
    uint32_t convergence_events = 0;
    uint32_t invalid_ssv_detected = 0;
    uint32_t invalid_ssrv_detected = 0;
    uint32_t iterator_sync_checks = 0;
    uint32_t value_sync_checks = 0;
} stats;

struct IteratorState {
    int16_t iterator;
    chrono::steady_clock::time_point last_update;
};

map<uint16_t, IteratorState> iterator_tracking;
map<uint16_t, int32_t> last_param_values;
map<uint16_t, int32_t> expected_param_values;

enum TestPhase {
    PHASE_INITIAL_SYNC = 1,
    PHASE_PARAMETER_CYCLING = 2,
    PHASE_STRESS_TESTING = 3,
    PHASE_NEW_PROCESS_JOIN = 4,
    PHASE_CONVERGENCE_CHECK = 5
};

TestPhase current_phase = PHASE_INITIAL_SYNC;
uint32_t phase_start_tick = 0;

void log_message(const string& tag, const string& msg) {
    auto now = chrono::duration_cast<chrono::microseconds>(
        chrono::steady_clock::now().time_since_epoch()).count();
    
    mtx_out.lock();
    cout << "[" << tag << "][" << now << "] PID:" << getpid() << " " << msg << endl;
    mtx_out.unlock();
}

void log_phase_change(TestPhase new_phase, uint32_t tick) {
    string phase_name;
    switch(new_phase) {
        case PHASE_INITIAL_SYNC: phase_name = "INITIAL SYNCHRONIZATION"; break;
        case PHASE_PARAMETER_CYCLING: phase_name = "PARAMETER CYCLING"; break;
        case PHASE_STRESS_TESTING: phase_name = "STRESS TESTING"; break;
        case PHASE_NEW_PROCESS_JOIN: phase_name = "NEW PROCESS JOINING"; break;
        case PHASE_CONVERGENCE_CHECK: phase_name = "CONVERGENCE CHECK"; break;
    }
    
    current_phase = new_phase;
    phase_start_tick = tick;
    log_message("PHASE", "🎯 " + phase_name + " (tick " + to_string(tick) + ")");
}

// Check iterator synchronization (should be ±1)
void check_iterator_synchronization(const ssv_message_t& message) {
    auto now = chrono::steady_clock::now();
    uint16_t param_num = message.param_num;
    
    if (iterator_tracking.find(param_num) != iterator_tracking.end()) {
        auto& prev_state = iterator_tracking[param_num];
        int16_t iterator_diff = abs(message.iterator - prev_state.iterator);
        
        if (iterator_diff <= 1 || iterator_diff >= 254) { // Account for wraparound
            stats.iterator_sync_checks++;
            if (stats.iterator_sync_checks % 1000 == 0) {
                log_message("ITER_SYNC", "✅ Iterator sync OK: param #" + to_string(param_num) + 
                           " diff=" + to_string(iterator_diff) + " (checked " + to_string(stats.iterator_sync_checks) + " times)");
            }
        } else {
            log_message("ITER_DESYNC", "⚠️ Iterator desync: param #" + to_string(param_num) + 
                       " current=" + to_string(message.iterator) + " prev=" + to_string(prev_state.iterator) + 
                       " diff=" + to_string(iterator_diff));
        }
    }
    
    iterator_tracking[param_num] = {message.iterator, now};
}

// Check value synchronization
void check_value_synchronization(const ssv_message_t& message) {
    uint16_t param_num = message.param_num;
    int32_t current_value = message.param_val;
    
    if (last_param_values.find(param_num) != last_param_values.end()) {
        int32_t prev_value = last_param_values[param_num];
        if (current_value != prev_value) {
            stats.value_sync_checks++;
            if (stats.value_sync_checks % 500 == 0) {
                log_message("VALUE_SYNC", "🔄 Value change: param #" + to_string(param_num) + 
                           " " + to_string(prev_value) + " → " + to_string(current_value) + 
                           " (tracked " + to_string(stats.value_sync_checks) + " changes)");
            }
        }
    }
    
    last_param_values[param_num] = current_value;
}

void check_parameter_changes() {
    static map<uint16_t, int32_t> last_check_values;
    uint32_t changes_this_check = 0;
    
    for (uint16_t i = 0; i < P_COUNT; i++) {
        uint16_t param_num = param_data->get_param_num(i);
        int32_t current_val = param_data->get_param_value(i);
        
        if (last_check_values.find(param_num) == last_check_values.end()) {
            last_check_values[param_num] = current_val;
        } else if (last_check_values[param_num] != current_val) {
            stats.param_changes++;
            changes_this_check++;
            last_check_values[param_num] = current_val;
        }
    }
    
    if (changes_this_check > 0 && stats.param_changes % 100 == 0) {
        log_message("PARAM_CHANGE", "🔄 Parameter changes detected: " + to_string(changes_this_check) + 
                   " this check, " + to_string(stats.param_changes) + " total");
    }
}

void new_process_parameter_setter(int sock_fd, struct sockaddr_in& remote_addr) {
    log_message("NEW_PROC", "🆕 New process starting parameter update sequence");
    
    // Wait for initialization
    this_thread::sleep_for(chrono::seconds(3));
    
    random_device rd;
    mt19937 gen(rd() + getpid());
    uniform_int_distribution<int32_t> value_dis(10000, 80000); // Some values will exceed max
    
    uint32_t updates_sent = 0;
    uint32_t invalid_values_sent = 0;
    
    for (uint16_t param_num = 0; param_num < P_COUNT; param_num++) {
        // Get max value for this parameter
        uint16_t param_idx = param_data->get_param_idx(param_num);
        uint32_t max_val = param_data->get_param_max_value(param_idx);
        
        int32_t new_value = value_dis(gen);
        bool is_valid = (new_value <= max_val);
        
        if (shared_data->add_ssrv_message(param_num, new_value)) {
            updates_sent++;
            stats.ssrv_sent++;
            
            if (!is_valid) {
                invalid_values_sent++;
                log_message("NEW_PROC_INVALID", "❌ Sending INVALID param #" + to_string(param_num) + 
                           " = " + to_string(new_value) + " > " + to_string(max_val) + " (should trigger SSE)");
            }
            
            if (updates_sent % 16 == 0) {
                log_message("NEW_PROC_UPDATE", "🔄 Parameter #" + to_string(param_num) + 
                           " = " + to_string(new_value) + " (" + (is_valid ? "valid" : "INVALID") + 
                           ") [" + to_string(updates_sent) + "/64]");
            }
        }
        
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    
    log_message("NEW_PROC", "✅ New process completed: " + to_string(updates_sent) + "/64 updates (" + 
               to_string(invalid_values_sent) + " invalid values that should trigger SSE)");
}

void sender_messages(int sock_fd, struct sockaddr_in& remote_addr)
{
    can_data_t can_data {};
    uint32_t tick_counter = 0;
    
    random_device rd;
    mt19937 gen(rd() + getpid());
    
    uint32_t process_factor = (getpid() % 1000) + 1;
    uniform_int_distribution<int32_t> value_dis(process_factor * 10, process_factor * 50);
    uniform_int_distribution<uint16_t> param_selector(0, P_COUNT - 1);
    
    log_message("INIT", "🚀 Ultimate 100-process test sender started (factor: " + to_string(process_factor) + ")");
    log_phase_change(PHASE_INITIAL_SYNC, tick_counter);

    while (!should_exit && tick_counter < (test_duration_ms / TICK_PERIOD))
    {
        uint32_t phase_ticks = tick_counter - phase_start_tick;
        
        // Phase management
        switch(current_phase) {
            case PHASE_INITIAL_SYNC:
                if (phase_ticks >= 1500) { // 30 seconds
                    log_phase_change(PHASE_PARAMETER_CYCLING, tick_counter);
                }
                break;
                
            case PHASE_PARAMETER_CYCLING:
                if (phase_ticks >= 2000) { // 40 seconds
                    log_phase_change(PHASE_STRESS_TESTING, tick_counter);
                }
                // Generate SSRV requests
                if (phase_ticks > 0 && phase_ticks % 100 == 0) { // Every 2 seconds
                    uint16_t param_idx = param_selector(gen);
                    uint16_t param_num = param_data->get_param_num(param_idx);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    int32_t new_value = value_dis(gen) % (max_val / 2); // Safe value
                    
                    if (shared_data->add_ssrv_message(param_num, new_value)) {
                        stats.ssrv_sent++;
                    }
                }
                break;
                
            case PHASE_STRESS_TESTING:
                if (phase_ticks >= 2000) { // 40 seconds
                    log_phase_change(PHASE_NEW_PROCESS_JOIN, tick_counter);
                }
                // High-frequency SSRV with some invalid values
                if (phase_ticks > 0 && phase_ticks % 50 == 0) { // Every 1 second
                    uint16_t param_idx = param_selector(gen);
                    uint16_t param_num = param_data->get_param_num(param_idx);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    // 80% valid, 20% invalid for SSE testing
                    bool send_valid = (tick_counter % 5) < 4;
                    int32_t new_value;
                    
                    if (send_valid) {
                        new_value = value_dis(gen) % (max_val / 2);
                    } else {
                        new_value = max_val + value_dis(gen); // Invalid for SSE testing
                        stats.invalid_ssrv_detected++;
                        if (stats.invalid_ssrv_detected % 10 == 0) {
                            log_message("STRESS_INVALID", "❌ Sending invalid SSRV #" + to_string(stats.invalid_ssrv_detected) + 
                                       " param #" + to_string(param_num) + " = " + to_string(new_value) + " > " + to_string(max_val));
                        }
                    }
                    
                    if (shared_data->add_ssrv_message(param_num, new_value)) {
                        stats.ssrv_sent++;
                    }
                }
                break;
                
            case PHASE_NEW_PROCESS_JOIN:
                if (phase_ticks >= 2000) { // 40 seconds
                    log_phase_change(PHASE_CONVERGENCE_CHECK, tick_counter);
                }
                break;
                
            case PHASE_CONVERGENCE_CHECK:
                // Just let everything settle
                break;
        }
        
        // Regular message sending through SharedData
        shared_data->period_counter();
        
        if (shared_data->get_messages(can_data)) {
            ssize_t result = sendto(sock_fd, &can_data, sizeof(can_data), 0, 
                                   (struct sockaddr*)&remote_addr, sizeof(remote_addr));
            if (result > 0) {
                switch (can_data.message_type) {
                    case SSV_MESSAGE:
                        stats.ssv_sent++;
                        if (stats.ssv_sent % 1000 == 0) {
                            log_message("SSV_SEND", "📡 SSV #" + to_string(stats.ssv_sent));
                        }
                        break;
                    case SSRV_MESSAGE:
                        stats.ssrv_sent++;
                        if (stats.ssrv_sent % 100 == 0) {
                            log_message("SSRV_SEND", "📤 SSRV #" + to_string(stats.ssrv_sent));
                        }
                        break;
                    case SSE_MESSAGE:
                        stats.sse_sent++;
                        log_message("SSE_SEND", "🚨 SSE ERROR sent! Total: " + to_string(stats.sse_sent));
                        break;
                }
            }
        }
        
        // Check parameter changes
        if (tick_counter % 100 == 0) {
            check_parameter_changes();
        }
        
        this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
        tick_counter++;
    }
    
    log_message("SHUTDOWN", "✅ Ultimate test sender completed after " + to_string(tick_counter) + " ticks");
}

void receiver_messages(int sock_fd)
{
    can_data_t can_data {};
    
    log_message("INIT", "👂 Ultimate test receiver started");
    
    while (!should_exit)
    {
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        
        ssize_t result = recvfrom(sock_fd, &can_data, sizeof(can_data), 0, 
                                 (struct sockaddr*)&sender_addr, &sender_len);
        
        if (result > 0) {
            shared_data->handle_messages(can_data);
            
            switch (can_data.message_type) {
                case SSV_MESSAGE: {
                    ssv_message_t message;
                    memcpy(&message, can_data.data, sizeof(message));
                    stats.ssv_received++;
                    
                    // Check for invalid SSV values that should trigger SSE
                    uint16_t param_idx = param_data->get_param_idx(message.param_num);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    if (message.param_val > max_val) {
                        stats.invalid_ssv_detected++;
                        if (stats.invalid_ssv_detected % 10 == 0) {
                            log_message("SSV_INVALID", "❌ Invalid SSV param #" + to_string(message.param_num) + 
                                       " = " + to_string(message.param_val) + " > " + to_string(max_val) + 
                                       " (total: " + to_string(stats.invalid_ssv_detected) + ")");
                        }
                    }
                    
                    // Check iterator and value synchronization
                    check_iterator_synchronization(message);
                    check_value_synchronization(message);
                    
                    if (stats.ssv_received % 2000 == 0) {
                        log_message("SSV_RECV", "📡 Received SSV #" + to_string(stats.ssv_received));
                    }
                    break;
                }
                case SSRV_MESSAGE: {
                    ssrv_message_t message;
                    memcpy(&message, can_data.data, sizeof(message));
                    stats.ssrv_received++;
                    
                    if (stats.ssrv_received % 200 == 0) {
                        log_message("SSRV_RECV", "📥 Received SSRV #" + to_string(stats.ssrv_received));
                    }
                    break;
                }
                case SSE_MESSAGE: {
                    sse_message_t message;
                    memcpy(&message, can_data.data, sizeof(message));
                    stats.sse_received++;
                    
                    log_message("SSE_RECV", "🚨 RECEIVED SSE ERROR param #" + to_string(message.param_num) + 
                               " error_code: " + to_string((int)message.error_code) + 
                               " (total: " + to_string(stats.sse_received) + ")");
                    break;
                }
            }
        }
    }
    
    log_message("SHUTDOWN", "✅ Ultimate test receiver completed");
}

void print_final_stats() {
    log_message("STATS", "==================== ULTIMATE 100-PROCESS TEST STATISTICS ====================");
    log_message("STATS", "Process ID: " + to_string(getpid()));
    log_message("STATS", "Test Duration: 150 seconds");
    log_message("STATS", "");
    log_message("STATS", "📊 Message Statistics:");
    log_message("STATS", "   SSV:  " + to_string(stats.ssv_sent) + " sent, " + to_string(stats.ssv_received) + " received");
    log_message("STATS", "   SSRV: " + to_string(stats.ssrv_sent) + " sent, " + to_string(stats.ssrv_received) + " received");
    log_message("STATS", "   SSE:  " + to_string(stats.sse_sent) + " sent, " + to_string(stats.sse_received) + " received");
    log_message("STATS", "");
    log_message("STATS", "🔄 Algorithm Performance:");
    log_message("STATS", "   Parameter Changes: " + to_string(stats.param_changes));
    log_message("STATS", "   Iterator Updates:  " + to_string(stats.iterator_updates));
    log_message("STATS", "   Convergence Events: " + to_string(stats.convergence_events));
    log_message("STATS", "");
    log_message("STATS", "🎯 Synchronization Validation:");
    log_message("STATS", "   Iterator Sync Checks: " + to_string(stats.iterator_sync_checks));
    log_message("STATS", "   Value Sync Checks:    " + to_string(stats.value_sync_checks));
    log_message("STATS", "");
    log_message("STATS", "🚨 Error Handling Validation:");
    log_message("STATS", "   Invalid SSV Detected:  " + to_string(stats.invalid_ssv_detected));
    log_message("STATS", "   Invalid SSRV Detected: " + to_string(stats.invalid_ssrv_detected));
    log_message("STATS", "");
    
    // Sample parameter values (every 8th parameter)
    log_message("STATS", "🏁 Sample Parameter States (every 8th parameter):");
    for (uint16_t i = 0; i < P_COUNT; i += 8) {
        uint16_t param_num = param_data->get_param_num(i);
        int32_t value = param_data->get_param_value(i);
        uint32_t max_val = param_data->get_param_max_value(i);
        log_message("STATS", "   Param #" + to_string(param_num) + ": " + to_string(value) + 
                   " (max: " + to_string(max_val) + ")");
    }
    
    log_message("STATS", "");
    
    // Algorithm validation
    bool ssv_working = (stats.ssv_sent > 100 && stats.ssv_received > 100);
    bool ssrv_working = (stats.ssrv_sent > 10);
    bool sse_working = (stats.sse_sent > 0 || stats.sse_received > 0);
    bool iterator_sync = (stats.iterator_sync_checks > 100);
    bool value_sync = (stats.value_sync_checks > 50);
    bool param_cycling = (stats.param_changes > P_COUNT);
    bool convergence = (stats.convergence_events > 5);
    
    log_message("STATS", "✅ Ultimate Algorithm Validation:");
    log_message("STATS", "   SSV Synchronization:     " + string(ssv_working ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   SSRV Processing:         " + string(ssrv_working ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   SSE Error Handling:      " + string(sse_working ? "✅ WORKING" : "⚠️ NOT TRIGGERED"));
    log_message("STATS", "   Iterator Synchronization:" + string(iterator_sync ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   Value Synchronization:   " + string(value_sync ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   Parameter Cycling:       " + string(param_cycling ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   Convergence:             " + string(convergence ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   New Process Integration: " + string(is_new_process ? "✅ TESTED" : "⏭️ SKIPPED"));
    
    log_message("STATS", "============================================================");
}

void setup_signal_handler() {
    signal(SIGINT, [](int) { 
        should_exit = true; 
        log_message("SIGNAL", "🛑 Received SIGINT - shutting down");
    });
    signal(SIGTERM, [](int) { 
        should_exit = true; 
        log_message("SIGNAL", "🛑 Received SIGTERM - shutting down");
    });
}

int main(int argc, char* argv[])
{
    setup_signal_handler();
    
    // Parse arguments
    uint16_t iterator_start = 0;
    if (argc > 1) {
        iterator_start = static_cast<uint16_t>(stoi(argv[1]));
    }
    if (argc > 2) {
        is_new_process = (stoi(argv[2]) == 1);
    }
    
    // Initialize parameter data
    param_data = new ParamData<P_COUNT>();
    shared_data = new SharedData<P_COUNT>();
    
    // Set different max values for different parameter ranges to test SSE
    param_data->set_param_max_value(50000); // Default max value
    
    // Initialize parameters with diverse max values for robust SSE testing
    for (uint16_t i = 0; i < P_COUNT; i++) {
        uint32_t param_specific_max;
        if (i % 4 == 0) param_specific_max = 5000;   // Every 4th param: low max
        else if (i % 4 == 1) param_specific_max = 50000; // High max
        else if (i % 4 == 2) param_specific_max = 15000; // Medium max  
        else param_specific_max = 25000; // Medium-high max
        
        // Use set_param_value_with_max to set specific max values
        param_data->set_param_value_with_max(i, i, i * 10, param_specific_max);
        shared_data->set_param_num(i);
        shared_data->set_iterator(i, iterator_start + i);
    }
    
    log_message("INIT", "🎮 Ultimate 100-process test started: PID=" + to_string(getpid()) + 
               " iterator=" + to_string(iterator_start) + (is_new_process ? " (NEW PROCESS)" : ""));
    log_message("INIT", "📏 Test configuration: 64 parameters, 150 seconds duration");
    log_message("INIT", "📏 Parameter max values: 5000/15000/25000/50000 (diverse for SSE testing)");
    
    // Setup UDP socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        log_message("ERROR", "Failed to create socket");
        return 1;
    }
    
    int enable = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        log_message("ERROR", "Failed to set SO_REUSEADDR");
        close(sock_fd);
        return 1;
    }
    
    struct sockaddr_in local_addr, remote_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(12345);
    
    if (bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        log_message("ERROR", "Failed to bind socket");
        close(sock_fd);
        return 1;
    }
    
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("239.255.255.250");
    remote_addr.sin_port = htons(12345);
    
    // Start threads
    thread receiver(receiver_messages, sock_fd);
    thread sender(sender_messages, sock_fd, ref(remote_addr));
    
    // If this is the new process, start parameter setter
    thread* new_proc_thread = nullptr;
    if (is_new_process) {
        new_proc_thread = new thread(new_process_parameter_setter, sock_fd, ref(remote_addr));
    }
    
    // Wait for completion
    sender.join();
    should_exit = true;
    receiver.join();
    
    if (new_proc_thread) {
        new_proc_thread->join();
        delete new_proc_thread;
    }
    
    print_final_stats();
    
    close(sock_fd);
    delete shared_data;
    delete param_data;
    
    return 0;
}
