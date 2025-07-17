#include "hdrs/shared_data.hpp"
#include "hdrs/client_server_shared_setpoint.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

bool running = true;
std::mutex mtx_out;  // Define the mutex that's declared extern in p_iterator.hpp

// Define the global variables that are declared extern in the headers
ParamData<P_COUNT>* param_data = nullptr;
SharedData<P_COUNT>* shared_data = nullptr;

void handle_stdin_commands()
{
    string line;
    while (running && getline(cin, line))
    {
        if (line.empty()) continue;
        
        istringstream iss(line);
        string command;
        iss >> command;
        
        if (command == "SSRV")
        {
            int param_num, new_value;
            if (iss >> param_num >> new_value)
            {
                mtx_out.lock();
                cout << "Received SSRV command: Change param " << param_num 
                     << " to " << new_value << endl;
                mtx_out.unlock();
                
                // Add SSRV message to shared_data
                if (shared_data) {
                    bool success = shared_data->add_ssrv_message(param_num, new_value);
                    if (success) {
                        mtx_out.lock();
                        cout << "SSRV message added successfully" << endl;
                        mtx_out.unlock();
                    }
                }
            }
        }
        else if (command == "QUIT")
        {
            running = false;
            break;
        }
    }
}

void check_param_data(ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
    for (uint16_t i = 0; i < P_COUNT; ++i)
    {
        if (param_data->get_param_value(i) != old_param_data->get_param_value(i))
        {
            mtx_out.lock();
            cout << "Param " << param_data->get_param_num(i) << " changed from "
                 << old_param_data->get_param_value(i) << " to "
                 << param_data->get_param_value(i) << endl;
            mtx_out.unlock();
            old_param_data->set_param_value(i, param_data->get_param_num(i), param_data->get_param_value(i));
        }
    }
}

void print_param_data(ParamData<P_COUNT> *param_data)
{
    for (uint16_t i = 0; i < P_COUNT; ++i)
    {
        uint16_t param_num = param_data->get_param_num(i);
        // Print ALL parameters, including param 0
        if (param_data->get_param_value(i) != 0) {  // Check if parameter has a value instead
            cout << "Param " << param_num << ": " << param_data->get_param_value(i) << endl;
        }
    }
}

void print_current_params(ParamData<P_COUNT> *param_data)
{
    mtx_out.lock();
    for (uint16_t i = 0; i < P_COUNT; ++i)
    {
        uint16_t param_num = param_data->get_param_num(i);
        // Print ALL parameters, including param 0
        if (param_data->get_param_value(i) != 0) {  // Check if parameter has a value instead
            cout << "Param " << param_num << ": " << param_data->get_param_value(i) << endl;
        }
    }
    mtx_out.unlock();
}

void main_loop()
{
    int tick_count = 0;
    ParamData<P_COUNT> old_param_data = *param_data;  // Create a copy to track changes
    
    mtx_out.lock();
    cout << "Starting main loop..." << endl;
    mtx_out.unlock();
    
    while (running)
    {
        // Increment period counter (simulates tick) - this processes SSRV messages
        if (shared_data) {
            shared_data->period_counter();
        }
        
        // Check for parameter changes and print them
        check_param_data(param_data, &old_param_data);
        
        // Print iterator value every 10 ticks (1 second)
        if (tick_count % 10 == 0)
        {
            mtx_out.lock();
            cout << "ITERATOR: " << tick_count / 10 << endl;
            mtx_out.unlock();
        }
        
        // Print current parameter values every 50 ticks (5 seconds) for orchestrator to capture
        if (tick_count % 50 == 0 && tick_count > 0)
        {
            print_current_params(param_data);
        }
        
        tick_count++;
        this_thread::sleep_for(chrono::milliseconds(100)); // 100ms
    }
    
    mtx_out.lock();
    cout << "Main loop ended" << endl;
    mtx_out.unlock();
}

void init_param_data(ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data, uint16_t step_kef)
{
    int32_t param_value = 0;

    for (uint16_t i = 0; i < P_COUNT; ++i)
    {
        param_value = static_cast<int32_t>(std::rand() % 9999 + 2077);
        param_data->set_param_value(i, i * step_kef, param_value);
        old_param_data->set_param_value(i, i * step_kef, param_value);
    }
}

void init_shared_data(SharedData<P_COUNT> *shared_data,
                      ParamData<P_COUNT> *param_data,
                      int16_t i_start_value = 0)
{
    for (uint16_t i = 0; i < P_COUNT; ++i)
    {
        uint16_t param_num = param_data->get_param_num(i);
        // Initialize ALL parameters, including param 0
        if (param_data->get_param_value(i) != 0) {  // Check if parameter has a value instead
            shared_data->set_param_num(param_num);
            shared_data->set_iterator(i, i_start_value);
        }
    }
}

int main(int argc, char** argv)
{
    int16_t iterator_start = 0;
    uint16_t param_kef = 1;
    
    if (argc > 1) iterator_start = atoi(argv[1]);
    if (argc > 2) param_kef = atoi(argv[2]);
    
    cout << "PID: " << getpid() << endl;
    cout << "Iterator start: " << iterator_start << endl;
    cout << "Param kef: " << param_kef << endl;
    
    // Initialize data structures
    param_data = new ParamData<P_COUNT>();
    ParamData<P_COUNT>* old_param_data = new ParamData<P_COUNT>();
    shared_data = new SharedData<P_COUNT>();
    
    // Seed random number generator with current time and process ID
    srand(time(NULL) + getpid());
    
    // Initialize parameters with random values (each process gets different values due to PID in seed)
    init_param_data(param_data, old_param_data, param_kef);
    
    // Initialize shared data
    init_shared_data(shared_data, param_data, iterator_start);
    
    // Print initial parameter values
    print_param_data(param_data);
    
    // Start stdin handler thread
    thread stdin_thread(handle_stdin_commands);
    stdin_thread.detach();
    
    // Run main loop - this will process SSRV messages and show parameter changes
    main_loop();
    
    // Cleanup
    delete param_data;
    delete old_param_data;
    delete shared_data;
    
    return 0;
}