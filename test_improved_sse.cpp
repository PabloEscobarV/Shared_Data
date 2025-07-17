/* ************************************************************************** */
/*                                                                            */
/*   test_improved_sse.cpp - Enhanced SSE and Parameter Diversity Test       */
/*   Tests with different MAX_VALUE per parameter for proper SSE validation  */
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

using namespace std;

// Global variables
ParamData<P_COUNT> *param_data = nullptr;
SharedData<P_COUNT> *shared_data = nullptr;
mutex mtx_out;

atomic<bool> should_exit(false);
uint32_t test_duration_ms = 30000; // 30 seconds
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
    uint32_t sse_triggered = 0;
} stats;

enum TestPhase {
    PHASE_SSV_SYNC = 1,
    PHASE_SSRV_VALID = 2,
    PHASE_SSE_TESTING = 3,
    PHASE_MIXED_TRAFFIC = 4
};

TestPhase current_phase = PHASE_SSV_SYNC;
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
        case PHASE_SSV_SYNC: phase_name = "SSV SYNCHRONIZATION"; break;
        case PHASE_SSRV_VALID: phase_name = "SSRV VALID REQUESTS"; break;
        case PHASE_SSE_TESTING: phase_name = "SSE ERROR TESTING"; break;
        case PHASE_MIXED_TRAFFIC: phase_name = "MIXED TRAFFIC"; break;
    }
    
    current_phase = new_phase;
    phase_start_tick = tick;
    log_message("PHASE", "🎯 Starting " + phase_name + " (tick " + to_string(tick) + ")");
}

void check_parameter_changes() {
    static map<uint16_t, int32_t> last_values;
    uint32_t changes_this_check = 0;
    
    for (uint16_t i = 0; i < P_COUNT; i++) {
        uint16_t param_num = param_data->get_param_num(i);
        int32_t current_val = param_data->get_param_value(i);
        uint32_t max_val = param_data->get_param_max_value(i);
        
        if (last_values.find(param_num) == last_values.end()) {
            last_values[param_num] = current_val;
        } else if (last_values[param_num] != current_val) {
            stats.param_changes++;
            changes_this_check++;
            log_message("PARAM_CHANGE", "Parameter #" + to_string(param_num) + 
                       " changed: " + to_string(last_values[param_num]) + " → " + to_string(current_val) +
                       " (max: " + to_string(max_val) + ")");
            last_values[param_num] = current_val;
        }
    }
}

void print_final_stats() {
    log_message("STATS", "==================== FINAL STATISTICS ====================");
    log_message("STATS", "SSV Messages: " + to_string(stats.ssv_sent) + " sent, " + 
                to_string(stats.ssv_received) + " received");
    log_message("STATS", "SSRV Messages: " + to_string(stats.ssrv_sent) + " sent, " + 
                to_string(stats.ssrv_received) + " received");
    log_message("STATS", "SSE Messages: " + to_string(stats.sse_sent) + " sent, " + 
                to_string(stats.sse_received) + " received");
    log_message("STATS", "Performance:");
    log_message("STATS", "   Parameter Changes: " + to_string(stats.param_changes));
    log_message("STATS", "   Iterator Updates:  " + to_string(stats.iterator_updates));
    log_message("STATS", "   SSE Triggered:     " + to_string(stats.sse_triggered));
    log_message("STATS", "");
    
    log_message("STATS", "🏁 Final Parameter States:");
    for (uint16_t i = 0; i < P_COUNT; i++) {
        uint16_t param_num = param_data->get_param_num(i);
        int32_t value = param_data->get_param_value(i);
        uint32_t max_val = param_data->get_param_max_value(i);
        log_message("STATS", "   Param #" + to_string(param_num) + ": " + to_string(value) + 
                   " (max: " + to_string(max_val) + ")");
    }
    log_message("STATS", "");
    
    // Algorithm validation
    bool ssv_working = (stats.ssv_sent > 0 && stats.ssv_received > 0);
    bool ssrv_working = (stats.ssrv_sent > 0 && stats.ssrv_received > 0);
    bool sse_working = (stats.sse_sent > 0 || stats.sse_received > 0);
    bool param_updates = (stats.param_changes > 0);
    
    log_message("STATS", "✅ Algorithm Validation:");
    log_message("STATS", "   SSV Synchronization: " + string(ssv_working ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   SSRV Processing:     " + string(ssrv_working ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   SSE Error Handling:  " + string(sse_working ? "✅ WORKING" : "❌ FAILED"));
    log_message("STATS", "   Parameter Updates:   " + string(param_updates ? "✅ WORKING" : "❌ FAILED"));
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

void sender_messages(int sock_fd, struct sockaddr_in& remote_addr)
{
    can_data_t can_data {};
    uint32_t tick_counter = 0;
    
    // Setup different random generators for different value ranges per process
    random_device rd;
    mt19937 gen(rd() + getpid()); // Different seed per process
    
    // Different value ranges based on process ID to create conflicts
    uint32_t process_factor = (getpid() % 1000) + 1;
    uniform_int_distribution<int32_t> value_dis(process_factor * 100, process_factor * 300);
    uniform_int_distribution<int32_t> high_value_dis(50000, 150000); // For SSE testing
    
    log_message("INIT", "🚀 Sender started (process factor: " + to_string(process_factor) + ")");
    log_message("INIT", "Value range: " + to_string(process_factor * 100) + " - " + to_string(process_factor * 300));
    
    // Print parameter max values
    for (uint16_t i = 0; i < P_COUNT; i++) {
        uint16_t param_num = param_data->get_param_num(i);
        uint32_t max_val = param_data->get_param_max_value(i);
        log_message("INIT", "Parameter #" + to_string(param_num) + " max value: " + to_string(max_val));
    }
    
    log_phase_change(PHASE_SSV_SYNC, tick_counter);

    while (!should_exit && tick_counter < (test_duration_ms / TICK_PERIOD))
    {
        uint32_t phase_ticks = tick_counter - phase_start_tick;
        
        // Phase management
        switch(current_phase) {
            case PHASE_SSV_SYNC:
                // First 8 seconds: SSV synchronization only
                if (phase_ticks >= 400) { // 8 seconds
                    log_phase_change(PHASE_SSRV_VALID, tick_counter);
                }
                break;
                
            case PHASE_SSRV_VALID:
                // Next 8 seconds: Valid SSRV requests (within max values)
                if (phase_ticks >= 400) { // 8 seconds  
                    log_phase_change(PHASE_SSE_TESTING, tick_counter);
                }
                // Generate valid SSRV requests
                if (phase_ticks > 0 && phase_ticks % 100 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    uint16_t param_num = param_data->get_param_num(param_idx);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    // Generate value within max limit for this parameter
                    int32_t safe_value = value_dis(gen) % (max_val / 2);  // Stay well below max
                    
                    if (shared_data->add_ssrv_message(param_num, safe_value)) {
                        stats.ssrv_sent++;
                        log_message("SSRV_VALID", "📤 Valid SSRV param #" + to_string(param_num) + 
                                   " = " + to_string(safe_value) + " (max: " + to_string(max_val) + ")");
                    }
                }
                break;
                
            case PHASE_SSE_TESTING:
                // Next 8 seconds: Invalid SSRV requests to trigger SSE
                if (phase_ticks >= 400) { // 8 seconds
                    log_phase_change(PHASE_MIXED_TRAFFIC, tick_counter);
                }
                // Generate invalid SSRV requests (exceed max values)
                if (phase_ticks > 0 && phase_ticks % 80 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    uint16_t param_num = param_data->get_param_num(param_idx);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    // Generate value that exceeds max limit
                    int32_t bad_value = max_val + high_value_dis(gen);
                    
                    if (shared_data->add_ssrv_message(param_num, bad_value)) {
                        stats.ssrv_sent++;
                        stats.sse_triggered++;
                        log_message("SSRV_BAD", "❌ Invalid SSRV param #" + to_string(param_num) + 
                                   " = " + to_string(bad_value) + " (max: " + to_string(max_val) + ") - SHOULD TRIGGER SSE");
                    }
                }
                break;
                
            case PHASE_MIXED_TRAFFIC:
                // Remaining time: Mixed valid and invalid traffic
                if (phase_ticks % 150 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    uint16_t param_num = param_data->get_param_num(param_idx);
                    uint32_t max_val = param_data->get_param_max_value(param_idx);
                    
                    // 70% valid, 30% invalid
                    bool send_valid = (tick_counter % 10) < 7;
                    int32_t new_value;
                    
                    if (send_valid) {
                        new_value = value_dis(gen) % (max_val / 2);
                        if (shared_data->add_ssrv_message(param_num, new_value)) {
                            stats.ssrv_sent++;
                            log_message("SSRV_MIX_VALID", "🔀 Mixed valid SSRV param #" + to_string(param_num) + 
                                       " = " + to_string(new_value) + " (max: " + to_string(max_val) + ")");
                        }
                    } else {
                        new_value = max_val + high_value_dis(gen);
                        if (shared_data->add_ssrv_message(param_num, new_value)) {
                            stats.ssrv_sent++;
                            stats.sse_triggered++;
                            log_message("SSRV_MIX_INVALID", "❌ Mixed invalid SSRV param #" + to_string(param_num) + 
                                       " = " + to_string(new_value) + " (max: " + to_string(max_val) + ") - SHOULD TRIGGER SSE");
                        }
                    }
                }
                break;
        }
        
        // Get and send messages (standard message generation)
        if (shared_data->get_messages(can_data))
        {
            switch(can_data.message_type)
            {
                case SSV_MESSAGE:
                    stats.ssv_sent++;
                    if (stats.ssv_sent % 20 == 0) {
                        log_message("SSV_SEND", "📡 SSV batch #" + to_string(stats.ssv_sent / 20));
                    }
                    break;
                case SSRV_MESSAGE:
                    log_message("SSRV_SEND", "📤 Broadcasting SSRV message");
                    break;
                case SSE_MESSAGE:
                    stats.sse_sent++;
                    log_message("SSE_SEND", "❌ Broadcasting SSE error #" + to_string(stats.sse_sent));
                    break;
            }
            
            sendto(sock_fd, &can_data, sizeof(can_data), 0, 
                   (struct sockaddr*)&remote_addr, sizeof(remote_addr));
        }
        
        // Check for parameter changes every 50 ticks (1 second)
        if (tick_counter % 50 == 0) {
            check_parameter_changes();
        }
        
        tick_counter++;
        this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
    }
    
    should_exit = true;
    log_message("SHUTDOWN", "✅ Sender completed after " + to_string(tick_counter) + " ticks");
}

void receiver_messages(int sock_fd)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in remote_addr;
    can_data_t can_data {};

    // Set socket to non-blocking for clean shutdown
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    log_message("INIT", "👂 Receiver started");

    while (!should_exit)
    {
        ssize_t bytes_received = recvfrom(sock_fd, &can_data, sizeof(can_data), 0,
                                         (struct sockaddr*)&remote_addr, &addr_len);

        if (bytes_received > 0 && can_data.idx_can != getpid())
        {
            switch(can_data.message_type)
            {
                case SSV_MESSAGE:
                    stats.ssv_received++;
                    if (stats.ssv_received % 20 == 0) {
                        log_message("SSV_RECV", "📡 Received SSV batch #" + to_string(stats.ssv_received / 20));
                    }
                    break;
                    
                case SSRV_MESSAGE:
                    stats.ssrv_received++;
                    log_message("SSRV_RECV", "📥 Received SSRV request #" + to_string(stats.ssrv_received));
                    break;
                    
                case SSE_MESSAGE:
                    stats.sse_received++;
                    log_message("SSE_RECV", "⚠️  Received SSE error #" + to_string(stats.sse_received));
                    break;
            }
            
            can_data.idx = getpid();
            shared_data->handle_messages(can_data);
            
            // Check for parameter changes after each message
            check_parameter_changes();
        }
        else if (bytes_received < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            log_message("ERROR", "❌ Receive error: " + string(strerror(errno)));
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(1));
    }

    log_message("SHUTDOWN", "✅ Receiver completed");
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <iterator_value>" << endl;
        return 1;
    }

    uint8_t iterator_value = static_cast<uint8_t>(atoi(argv[1]));
    
    setup_signal_handler();
    
    // Initialize parameter data with different max values for SSE testing
    param_data = new ParamData<P_COUNT>();
    
    // Setup parameters with different max values:
    // Param #0: Low max value (easy to exceed for SSE testing)
    // Param #1: Medium max value  
    // Param #2: High max value (harder to exceed)
    uint32_t max_values[P_COUNT] = {5000, 25000, 99999};  // Different limits per parameter
    
    for (uint16_t i = 0; i < P_COUNT; ++i) {
        uint16_t param_num = i;
        uint32_t initial_value = param_num;  // Start with param number as value
        uint32_t max_val = max_values[i];
        
        param_data->set_param_value_with_max(i, param_num, initial_value, max_val);
        log_message("INIT", "Initialized param #" + to_string(param_num) + 
                   " = " + to_string(initial_value) + " (max: " + to_string(max_val) + ")");
    }
    
    // Initialize shared data
    shared_data = new SharedData<P_COUNT>();
    
    // Set iterator for all parameters
    for (uint16_t i = 0; i < P_COUNT; i++) {
        shared_data->set_iterator(i, iterator_value);
    }
    
    log_message("INIT", "🎮 Process started with iterator: " + to_string(static_cast<int>(iterator_value)));
    log_message("INIT", "🎯 Test focuses on SSE error handling with different max values per parameter");
    
    // Setup UDP socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        log_message("ERROR", "❌ Failed to create socket");
        return 1;
    }

    // Enable broadcast and allow reuse
    int broadcast = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in local_addr {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(12345);

    if (bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        log_message("ERROR", "❌ Failed to bind socket");
        close(sock_fd);
        return 1;
    }

    struct sockaddr_in remote_addr {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    remote_addr.sin_port = htons(12345);

    // Start receiver thread
    thread receiver_thread(receiver_messages, sock_fd);
    
    // Start sender (main thread)
    sender_messages(sock_fd, remote_addr);
    
    // Wait for receiver to finish
    receiver_thread.join();
    
    print_final_stats();
    
    close(sock_fd);
    delete shared_data;
    delete param_data;
    
    return 0;
}
