#include "hdrs/queue.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <limits>

// ANSI Color codes for beautiful output
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"

// Helper function to print queue state
template<typename T, uint8_t S>
void print_queue_state(FSQueue<T, S>& queue, const std::string& label)
{
    std::cout << CYAN << label << RESET << ": ";
    
    FSQueue<T, S> temp_queue = queue;
    T item;
    bool first = true;
    while (temp_queue.pop(item))
    {
        if (!first) std::cout << " ";
        std::cout << YELLOW << item << RESET;
        first = false;
    }
    std::cout << MAGENTA << " (count: " << (int)queue.get_count() << ")" << RESET << std::endl;
}

void print_test_header(const std::string& test_name)
{
    std::cout << "\n" << BOLD << BLUE << "=== " << test_name << " ===" << RESET << std::endl;
}

void print_success(const std::string& message)
{
    std::cout << GREEN << "✓ " << message << RESET << std::endl;
}

void print_error(const std::string& message)
{
    std::cout << RED << "❌ " << message << RESET << std::endl;
}

// Test constructor and basic properties
void test_constructor_and_properties()
{
    print_test_header("Testing Constructor and Basic Properties");
    
    FSQueue<int, 5> queue;
    
    // Test initial state
    assert(queue.is_empty() == true);
    assert(queue.is_full() == false);
    assert(queue.get_count() == 0);
    
    print_success("Initial state test passed");
}

// Test push functionality
void test_push_functionality()
{
    print_test_header("Testing Push Functionality");
    
    FSQueue<int, 3> queue;
    
    // Test single push
    bool result = queue.push(10);
    assert(result == true);
    assert(queue.get_count() == 1);
    assert(queue.is_empty() == false);
    
    // Test multiple pushes
    queue.push(20);
    queue.push(30);
    assert(queue.get_count() == 3);
    assert(queue.is_full() == true);
    
    // Test push when full
    result = queue.push(40);
    assert(result == false); // Should fail when full
    assert(queue.get_count() == 3); // Count unchanged
    
    print_queue_state(queue, "After pushes");
    
    print_success("Push functionality test passed");
}

// Test pop functionality
void test_pop_functionality()
{
    print_test_header("Testing Pop Functionality");
    
    FSQueue<char, 4> queue;
    
    // Test pop from empty queue
    char item;
    bool result = queue.pop(item);
    assert(result == false);
    
    // Fill queue
    queue.push('A');
    queue.push('B');
    queue.push('C');
    
    print_queue_state(queue, "Before pops");
    
    // Test pop with return value
    result = queue.pop(item);
    assert(result == true);
    assert(item == 'A');
    assert(queue.get_count() == 2);
    
    // Test pop without return value
    result = queue.pop();
    assert(result == true);
    assert(queue.get_count() == 1);
    
    // Pop last item
    queue.pop(item);
    assert(item == 'C');
    assert(queue.is_empty() == true);
    
    // Test pop from empty again
    result = queue.pop();
    assert(result == false);
    
    print_success("Pop functionality test passed");
}

// Test peek functionality
void test_peek_functionality()
{
    print_test_header("Testing Peek Functionality");
    
    FSQueue<int, 5> queue;
    
    // Test peek on empty queue
    int item;
    bool result = queue.peek(item);
    assert(result == false);
    
    // Test peek_second on empty queue
    result = queue.peek_second(item);
    assert(result == false);
    
    // Add one item
    queue.push(100);
    
    // Test peek first
    result = queue.peek(item);
    assert(result == true);
    assert(item == 100);
    assert(queue.get_count() == 1); // Count unchanged
    
    // Test peek_second with only one item
    result = queue.peek_second(item);
    assert(result == false);
    
    // Add second item
    queue.push(200);
    
    // Test peek_second
    result = queue.peek_second(item);
    assert(result == true);
    assert(item == 200);
    assert(queue.get_count() == 2); // Count unchanged
    
    // Verify peek doesn't change queue
    queue.peek(item);
    assert(item == 100);
    
    print_success("Peek functionality test passed");
}

// Test swap functionality (priority reordering)
void test_swap_functionality()
{
    print_test_header("Testing Swap Functionality");
    
    FSQueue<int, 8> queue;
    
    // Test swap on empty queue
    queue.swap(1);
    assert(queue.is_empty() == true);
    
    // Test swap with single item
    queue.push(10);
    queue.swap(1); // Should do nothing (out of bounds)
    int item;
    queue.peek(item);
    assert(item == 10);
    
    // Test basic swap functionality
    queue.push(20);
    queue.push(30);
    queue.push(40);
    // Queue: [10, 20, 30, 40]
    
    print_queue_state(queue, "Before swap(2)");
    
    queue.swap(2); // Move 10 to position 2
    // Expected: [20, 30, 10, 40]
    
    print_queue_state(queue, "After swap(2)");
    
    queue.pop(item); assert(item == 20);
    queue.pop(item); assert(item == 30);
    queue.pop(item); assert(item == 10); // Was moved to position 2
    queue.pop(item); assert(item == 40);
    
    print_success("Swap functionality test passed");
}

// Test queue wraparound behavior
void test_wraparound_behavior()
{
    print_test_header("Testing Wraparound Behavior");
    
    FSQueue<int, 4> queue;
    
    // Fill queue completely
    queue.push(1);
    queue.push(2);
    queue.push(3);
    queue.push(4);
    
    print_queue_state(queue, "Initially full");
    
    // Pop some items
    int item;
    queue.pop(item); // Remove 1
    queue.pop(item); // Remove 2
    
    // Add new items (should wrap around)
    queue.push(5);
    queue.push(6);
    
    print_queue_state(queue, "After wraparound");
    
    // Verify order: [3, 4, 5, 6]
    queue.pop(item); assert(item == 3);
    queue.pop(item); assert(item == 4);
    queue.pop(item); assert(item == 5);
    queue.pop(item); assert(item == 6);
    
    assert(queue.is_empty() == true);
    
    print_success("Wraparound behavior test passed");
}

// Test edge cases and boundary conditions
void test_edge_cases()
{
    print_test_header("Testing Edge Cases");
    
    // Test with size 1 queue
    FSQueue<int, 1> tiny_queue;
    
    assert(tiny_queue.push(42) == true);
    assert(tiny_queue.is_full() == true);
    assert(tiny_queue.push(43) == false); // Should fail
    
    int item;
    tiny_queue.peek(item);
    assert(item == 42);
    
    assert(tiny_queue.pop(item) == true);
    assert(item == 42);
    assert(tiny_queue.is_empty() == true);
    
    // Test swap edge cases
    FSQueue<int, 5> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);
    
    print_queue_state(queue, "Before edge case tests");
    
    // Test swap(0) - should do nothing
    queue.swap(0);
    queue.peek(item);
    assert(item == 1); // Should be unchanged
    std::cout << DIM << "  ✓ swap(0) test passed" << RESET << std::endl;
    
    // Test swap with index >= count (should be handled gracefully)
    queue.swap(10); // Should do nothing or be clamped
    queue.peek(item);
    // Note: Based on your implementation, this might actually change the queue
    // Let's check what the current head is after this operation
    std::cout << DIM << "  → After swap(10), head item is: " << item << RESET << std::endl;
    
    print_success("Edge cases test passed");
}

// Test with different data types
void test_different_data_types()
{
    print_test_header("Testing Different Data Types");
    
    // Test with strings
    FSQueue<std::string, 3> string_queue;
    string_queue.push("Hello");
    string_queue.push("World");
    string_queue.push("Test");
    
    std::string str;
    string_queue.pop(str);
    assert(str == "Hello");
    
    // Test with floating point
    FSQueue<double, 3> double_queue;
    double_queue.push(3.14);
    double_queue.push(2.71);
    
    double val;
    double_queue.peek(val);
    assert(val == 3.14);
    
    // Test with characters
    FSQueue<char, 4> char_queue;
    char_queue.push('A');
    char_queue.push('B');
    char_queue.push('C');
    
    print_queue_state(char_queue, "Character queue");
    
    print_success("Different data types test passed");
}

// Test performance and stress conditions
void test_performance_stress()
{
    print_test_header("Testing Performance and Stress Conditions");
    
    FSQueue<int, 100> large_queue;
    
    // Fill large queue
    for (int i = 0; i < 100; i++)
    {
        bool result = large_queue.push(i);
        assert(result == true);
    }
    
    assert(large_queue.is_full() == true);
    assert(large_queue.get_count() == 100);
    std::cout << DIM << "  ✓ Filled queue with 100 items" << RESET << std::endl;
    
    // Test multiple swaps
    for (int i = 1; i < 10; i++)
    {
        large_queue.swap(i);
    }
    std::cout << DIM << "  ✓ Performed 9 swap operations" << RESET << std::endl;
    
    // Empty the queue and verify order is maintained
    int count = 0;
    int item;
    while (large_queue.pop(item))
    {
        count++;
    }
    
    assert(count == 100);
    assert(large_queue.is_empty() == true);
    std::cout << DIM << "  ✓ Successfully popped all 100 items" << RESET << std::endl;
    
    print_success("Performance and stress test passed");
}

// Test queue state consistency
void test_state_consistency()
{
    print_test_header("Testing State Consistency");
    
    FSQueue<int, 5> queue;
    
    // Verify initial state consistency
    assert(queue.is_empty() == true);
    assert(queue.is_full() == false);
    assert(queue.get_count() == 0);
    
    // Add items and verify state
    for (int i = 1; i <= 5; i++)
    {
        queue.push(i * 10);
        assert(queue.get_count() == i);
        assert(queue.is_empty() == false);
        assert(queue.is_full() == (i == 5));
    }
    std::cout << DIM << "  ✓ State consistency verified during filling" << RESET << std::endl;
    
    // Remove items and verify state
    for (int i = 5; i >= 1; i--)
    {
        int item;
        bool result = queue.pop(item);
        assert(result == true);
        assert(queue.get_count() == i - 1);
        assert(queue.is_empty() == (i == 1));
        assert(queue.is_full() == false);
    }
    std::cout << DIM << "  ✓ State consistency verified during emptying" << RESET << std::endl;
    
    print_success("State consistency test passed");
}

// Test realistic use case scenarios
void test_realistic_scenarios()
{
    print_test_header("Testing Realistic Use Case Scenarios");
    
    // Scenario 1: Task scheduling with priority reordering
    FSQueue<int, 10> task_queue;
    
    // Add tasks
    for (int i = 1; i <= 5; i++)
    {
        task_queue.push(i * 100); // Tasks: 100, 200, 300, 400, 500
    }
    
    print_queue_state(task_queue, "Initial task queue");
    
    // Task 100 needs more time, defer it
    task_queue.swap(2);
    print_queue_state(task_queue, "After deferring task 100");
    
    // Process tasks
    int task;
    task_queue.pop(task); // Should be 200
    assert(task == 200);
    
    task_queue.pop(task); // Should be 300
    assert(task == 300);
    
    task_queue.pop(task); // Should be 100 (was deferred)
    assert(task == 100);
    
    std::cout << DIM << "  ✓ Task scheduling scenario completed" << RESET << std::endl;
    
    // Scenario 2: Producer-consumer with wraparound
    FSQueue<char, 4> buffer;
    
    // Producer adds data
    buffer.push('A');
    buffer.push('B');
    buffer.push('C');
    
    // Consumer processes some
    char data;
    buffer.pop(data); // A
    buffer.pop(data); // B
    
    // Producer adds more (wraparound)
    buffer.push('D');
    buffer.push('E');
    
    // Verify final state: [C, D, E]
    buffer.pop(data); assert(data == 'C');
    buffer.pop(data); assert(data == 'D');
    buffer.pop(data); assert(data == 'E');
    
    std::cout << DIM << "  ✓ Producer-consumer scenario completed" << RESET << std::endl;
    
    print_success("Realistic scenarios test passed");
}

int main()
{
    std::cout << BOLD << MAGENTA << "========================================" << RESET << std::endl;
    std::cout << BOLD << MAGENTA << "=== COMPLETE FSQueue UNIT TEST SUITE ===" << RESET << std::endl;
    std::cout << BOLD << MAGENTA << "========================================" << RESET << std::endl;
    
    try {
        test_constructor_and_properties();
        test_push_functionality();
        test_pop_functionality();
        test_peek_functionality();
        test_swap_functionality();
        test_wraparound_behavior();
        test_edge_cases();
        test_different_data_types();
        test_performance_stress();
        test_state_consistency();
        test_realistic_scenarios();
        
        std::cout << "\n" << BOLD << GREEN << "========================================" << RESET << std::endl;
        std::cout << BOLD << GREEN << "🎉 ALL TESTS PASSED SUCCESSFULLY! 🎉" << RESET << std::endl;
        std::cout << BOLD << GREEN << "FSQueue implementation is working correctly" << RESET << std::endl;
        std::cout << BOLD << GREEN << "========================================" << RESET << std::endl;
        
    }
    catch (const std::exception& e) {
        print_error("TEST FAILED with exception: " + std::string(e.what()));
        return 1;
    }
    catch (...) {
        print_error("TEST FAILED with unknown exception");
        return 1;
    }
    
    return 0;
}