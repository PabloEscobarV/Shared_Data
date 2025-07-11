#include "../hdrs/shared_data.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>

// Integration test for SharedData class
int main()
{
    constexpr uint16_t COUNT = 5;
    SharedData<COUNT> shared;
    std::atomic<bool> running(true);

    // Task: generate SSV messages every 100ms
    auto ssv_task = [&]() {
        while (running.load())
        {
            shared.period_counter();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    // Task: randomly add SSRV messages
    auto ssrv_task = [&]() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint16_t> dist_param(0, COUNT - 1);
        std::uniform_int_distribution<int32_t> dist_val(-100, 100);

        while (running.load())
        {
            uint16_t param = dist_param(rng);
            int32_t val = dist_val(rng);
            shared.add_ssrv_message(param, val);
            std::this_thread::sleep_for(std::chrono::milliseconds((dist_val(rng) % 100) + 50));
        }
    };

    // Task: process messages frequently
    auto process_task = [&]() {
        while (running.load())
        {
            shared.get_messages();
            shared.handle_messages();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    // Start threads
    std::thread t1(ssv_task);
    std::thread t2(ssrv_task);
    std::thread t3(process_task);

    // Let it run for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    running.store(false);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "Test completed successfully." << std::endl;
    return 0;
}
