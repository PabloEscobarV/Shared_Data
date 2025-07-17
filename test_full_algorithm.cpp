/* ************************************************************************** */
/*                                                                            */
/*   test_full_algorithm.cpp - Comprehensive Algorithm Test                  */
/*   Tests SSV, SSRV, and SSE message workflows                              */
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

using namespace std;

// Global variables
ParamData<P_COUNT> *param_data = nullptr;
SharedData<P_COUNT> *shared_data = nullptr;
mutex mtx_out;

atomic<bool> should_exit(false);
uint32_t test_duration_ms = 45000; // 45 seconds for comprehensive test
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
} stats;

enum TestPhase {
    PHASE_SSV_SYNC = 1,
    PHASE_SSRV_REQUESTS = 2,
    PHASE_SSE_ERRORS = 3,
    PHASE_MIXED_TRAFFIC = 4
};

TestPhase current_phase = PHASE_SSV_SYNC;
uint32_t phase_start_tick = 0;

void signal_handler(int signum)
{
    cout << "\n🛑 Signal " << signum << " received. Shutting down..." << endl;
    should_exit = true;
}

void log_message(const string& category, const string& message)
{
    auto now = chrono::steady_clock::now();
    auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
    cout << "[" << category << "][" << timestamp << "] PID:" << getpid() << " " << message << endl;
}

void log_phase_change(TestPhase new_phase, uint32_t tick)
{
    string phase_name;
    switch(new_phase) {
        case PHASE_SSV_SYNC: phase_name = "SSV SYNCHRONIZATION"; break;
        case PHASE_SSRV_REQUESTS: phase_name = "SSRV REQUESTS"; break;
        case PHASE_SSE_ERRORS: phase_name = "SSE ERROR HANDLING"; break;
        case PHASE_MIXED_TRAFFIC: phase_name = "MIXED TRAFFIC"; break;
    }
    
    log_message("PHASE", "🎯 Starting " + phase_name + " (tick " + to_string(tick) + ")");
    current_phase = new_phase;
    phase_start_tick = tick;
}

void check_parameter_changes()
{
    static int32_t last_param_values[P_COUNT] = {0};
    static bool first_check = true;
    
    if (first_check) {
        for (uint16_t i = 0; i < P_COUNT; i++) {
            last_param_values[i] = param_data->get_param_value(i);
        }
        first_check = false;
        return;
    }
    
    for (uint16_t i = 0; i < P_COUNT; i++) {
        int32_t current_value = param_data->get_param_value(i);
        if (current_value != last_param_values[i]) {
            stats.param_changes++;
            log_message("PARAM_CHANGE", "Parameter #" + to_string(param_data->get_param_num(i)) +
                       " changed: " + to_string(last_param_values[i]) + " → " + to_string(current_value));
            last_param_values[i] = current_value;
        }
    }
}

void send_messages(int sock_fd, struct sockaddr_in& remote_addr, uint16_t process_id)
{
    can_data_t can_data {};
    uint32_t tick_counter = 0;
    uint32_t max_ticks = test_duration_ms / TICK_PERIOD;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> value_dis(1000, 8000);

    can_data.idx_can = process_id;
    can_data.idx = process_id;
    
    log_message("INIT", "🚀 Test sender started (Process ID: " + to_string(process_id) + ")");
    log_phase_change(PHASE_SSV_SYNC, tick_counter);
    
    while (tick_counter < max_ticks && !should_exit)
    {
        shared_data->period_counter();
        
        // Phase management
        uint32_t phase_ticks = tick_counter - phase_start_tick;
        
        switch(current_phase) {
            case PHASE_SSV_SYNC:
                // First 10 seconds: Only SSV messages for synchronization
                if (phase_ticks >= 500) { // 10 seconds
                    log_phase_change(PHASE_SSRV_REQUESTS, tick_counter);
                }
                break;
                
            case PHASE_SSRV_REQUESTS:
                // Next 15 seconds: Add SSRV requests  
                if (phase_ticks >= 750) { // 15 seconds
                    log_phase_change(PHASE_SSE_ERRORS, tick_counter);
                }
                // Generate SSRV requests every 2 seconds
                if (phase_ticks > 0 && phase_ticks % 100 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    int32_t new_value = value_dis(gen);
                    
                    if (shared_data->add_ssrv_message(param_data->get_param_num(param_idx), new_value)) {
                        stats.ssrv_sent++;
                        log_message("SSRV_REQ", "📤 Requesting param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(new_value));
                    } else {
                        log_message("SSRV_FAIL", "❌ Failed to add SSRV request for param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(new_value));
                    }
                }
                break;
                
            case PHASE_SSE_ERRORS:
                // Next 10 seconds: Test SSE error handling with out-of-range values
                if (phase_ticks >= 500) { // 10 seconds
                    log_phase_change(PHASE_MIXED_TRAFFIC, tick_counter);
                }
                // Generate SSRV with out-of-range values to trigger SSE
                if (phase_ticks > 0 && phase_ticks % 150 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    int32_t bad_value = 50000; // Should exceed max value
                    
                    if (shared_data->add_ssrv_message(param_data->get_param_num(param_idx), bad_value)) {
                        stats.ssrv_sent++;
                        log_message("SSRV_BAD", "❌ Requesting INVALID param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(bad_value));
                    } else {
                        log_message("SSRV_BAD_FAIL", "❌ Failed to add INVALID SSRV request for param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(bad_value));
                    }
                }
                break;
                
            case PHASE_MIXED_TRAFFIC:
                // Remaining time: Mixed traffic
                // SSRV requests
                if (phase_ticks % 200 == 0) {
                    uint16_t param_idx = tick_counter % P_COUNT;
                    int32_t new_value = value_dis(gen);
                    
                    if (shared_data->add_ssrv_message(param_data->get_param_num(param_idx), new_value)) {
                        stats.ssrv_sent++;
                        log_message("SSRV_MIX", "🔀 Mixed traffic SSRV param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(new_value));
                    } else {
                        log_message("SSRV_MIX_FAIL", "❌ Failed mixed traffic SSRV for param #" + 
                                   to_string(param_data->get_param_num(param_idx)) + " = " + to_string(new_value));
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
                    log_message("SSE_SEND", "❌ Broadcasting SSE error");
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
        int n = recvfrom(sock_fd, &can_data, sizeof(can_data), 0, 
                         (struct sockaddr*)&remote_addr, &addr_len);
        if (n > 0 && can_data.idx_can != getpid())
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
                    log_message("SSRV_RECV", "📤 Received SSRV #" + to_string(stats.ssrv_received));
                    break;
                case SSE_MESSAGE:
                    stats.sse_received++;
                    log_message("SSE_RECV", "❌ Received SSE error #" + to_string(stats.sse_received));
                    break;
            }
            
            can_data.idx = getpid();
            shared_data->handle_messages(can_data);
            
            // Check for parameter changes after each message
            check_parameter_changes();
        }
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            break; // Error
        }
        
        this_thread::sleep_for(chrono::microseconds(100));
    }
    
    log_message("SHUTDOWN", "👂 Receiver stopped");
}

void print_final_report()
{
    cout << "\n📊 ================ COMPREHENSIVE TEST REPORT ================" << endl;
    cout << "🏷️  Process ID: " << getpid() << endl;
    cout << "⏱️  Test Duration: " << test_duration_ms / 1000 << " seconds" << endl;
    
    cout << "\n📤 Messages Sent:" << endl;
    cout << "   SSV:  " << stats.ssv_sent << " (synchronization)" << endl;
    cout << "   SSRV: " << stats.ssrv_sent << " (parameter requests)" << endl;
    cout << "   SSE:  " << stats.sse_sent << " (error notifications)" << endl;
    
    cout << "\n📥 Messages Received:" << endl;
    cout << "   SSV:  " << stats.ssv_received << " (from other processes)" << endl;
    cout << "   SSRV: " << stats.ssrv_received << " (parameter requests)" << endl;
    cout << "   SSE:  " << stats.sse_received << " (error notifications)" << endl;
    
    cout << "\n🔄 Algorithm Performance:" << endl;
    cout << "   Parameter Changes: " << stats.param_changes << endl;
    cout << "   Iterator Updates:  " << stats.iterator_updates << endl;
    
    cout << "\n🏁 Final Parameter States:" << endl;
    for (uint16_t i = 0; i < P_COUNT; i++) {
        cout << "   Param #" << param_data->get_param_num(i) 
             << ": " << param_data->get_param_value(i) << endl;
    }
    
    cout << "\n✅ Algorithm Validation:" << endl;
    cout << "   SSV Synchronization: " << (stats.ssv_received > 0 ? "✅ WORKING" : "❌ FAILED") << endl;
    cout << "   SSRV Processing:     " << (stats.ssrv_received > 0 ? "✅ WORKING" : "❌ FAILED") << endl; 
    cout << "   SSE Error Handling:  " << (stats.sse_received > 0 ? "✅ WORKING" : "❌ FAILED") << endl;
    cout << "   Parameter Updates:   " << (stats.param_changes > 0 ? "✅ WORKING" : "❌ FAILED") << endl;
    
    cout << "============================================================\n" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <iterator_start> <param_kef> [duration_ms]" << endl;
        cerr << "  iterator_start: Initial iterator value (0-255)" << endl;
        cerr << "  param_kef: Parameter step coefficient" << endl;
        cerr << "  duration_ms: Test duration in milliseconds (default=45000)" << endl;
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int16_t iterator_start = stoi(argv[1]);
    int16_t param_kef = stoi(argv[2]);
    if (argc > 3) test_duration_ms = stoi(argv[3]);

    log_message("INIT", "🧪 COMPREHENSIVE ALGORITHM TEST STARTING");
    log_message("CONFIG", "Iterator: " + to_string(iterator_start) + 
                         ", ParamKef: " + to_string(param_kef) + 
                         ", Duration: " + to_string(test_duration_ms) + "ms");

    // Initialize data structures
    param_data = new ParamData<P_COUNT>();
    shared_data = new SharedData<P_COUNT>();

    for (uint16_t i = 0; i < P_COUNT; i++) {
        param_data->set_param_value(i, i, i * param_kef);
        shared_data->set_param_num(i);
        shared_data->set_iterator(i, iterator_start);
    }

    // Setup multicast socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in local_addr, remote_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    memset(&remote_addr, 0, sizeof(remote_addr));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(12345);

    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "239.255.255.250", &remote_addr.sin_addr);
    remote_addr.sin_port = htons(12345);

    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return 1;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

    // Start receiver thread
    thread receiver_thread(receiver_messages, sock_fd);

    // Run sender in main thread
    send_messages(sock_fd, remote_addr, getpid());

    // Wait for receiver to finish
    receiver_thread.join();

    print_final_report();

    close(sock_fd);
    delete param_data;
    delete shared_data;

    return 0;
}
