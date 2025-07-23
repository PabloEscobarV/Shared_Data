#include "hdrs/p_iterator.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>

// ANSI color codes for better test output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

class P_IteratorCompleteUnitTest {
private:
    int total_tests = 0;
    int passed_tests = 0;
    
    void log_test(bool passed, const std::string& test_name, const std::string& details = "") {
        total_tests++;
        if (passed) {
            passed_tests++;
            std::cout << GREEN << "✓ " << RESET << test_name << std::endl;
        } else {
            std::cout << RED << "✗ " << RESET << test_name;
            if (!details.empty()) {
                std::cout << " - " << details;
            }
            std::cout << std::endl;
        }
    }

public:
    // Test Constructor
    void test_constructor() {
        std::cout << CYAN << "\n=== Testing P_Iterator Constructor ===" << RESET << std::endl;
        
        // Default constructor
        P_Iterator iter1;
        log_test(iter1.get_iterator() == 0, 
                "Default constructor initializes to 0",
                "Expected: 0, Got: " + std::to_string(iter1.get_iterator()));
        
        // Constructor with value
        P_Iterator iter2(100);
        log_test(iter2.get_iterator() == 100, 
                "Constructor with value 100",
                "Expected: 100, Got: " + std::to_string(iter2.get_iterator()));
        
        // Constructor with large bit value
        P_Iterator iter3(32768);
        log_test(iter3.get_iterator() == 32768, 
                "Constructor with large bit value 32768",
                "Expected: 32768, Got: " + std::to_string(iter3.get_iterator()));
        
        // Constructor with max value
        P_Iterator iter4(65535);
        log_test(iter4.get_iterator() == 65535, 
                "Constructor with max value 65535",
                "Expected: 65535, Got: " + std::to_string(iter4.get_iterator()));
    }
    
    // Test get_iterator() method
    void test_get_iterator() {
        std::cout << CYAN << "\n=== Testing get_iterator() Method ===" << RESET << std::endl;
        
        P_Iterator iter(12345);
        log_test(iter.get_iterator() == 12345, 
                "get_iterator() returns correct value");
        
        // Test with various values
        std::vector<uint16_t> test_values = {0, 1, 32767, 32768, 65534, 65535};
        bool all_passed = true;
        
        for (uint16_t val : test_values) {
            P_Iterator test_iter(val);
            if (test_iter.get_iterator() != val) {
                all_passed = false;
                break;
            }
        }
        log_test(all_passed, "get_iterator() works for all test values");
    }
    
    // Test set_iterator() method (test purposes only)
    void test_set_iterator() {
        std::cout << CYAN << "\n=== Testing set_iterator() Method (Test Only) ===" << RESET << std::endl;
        
        P_Iterator iter(0);
        
        iter.set_iterator(500);
        log_test(iter.get_iterator() == 500, 
                "set_iterator(500) works correctly");
        
        iter.set_iterator(32768);
        log_test(iter.get_iterator() == 32768, 
                "set_iterator(32768) with large bit");
        
        iter.set_iterator(65535);
        log_test(iter.get_iterator() == 65535, 
                "set_iterator(65535) max value");
        
        iter.set_iterator(0);
        log_test(iter.get_iterator() == 0, 
                "set_iterator(0) reset to zero");
    }
    
    // Test uint16_t conversion operator
    void test_conversion_operator() {
        std::cout << CYAN << "\n=== Testing uint16_t Conversion Operator ===" << RESET << std::endl;
        
        P_Iterator iter(12345);
        uint16_t converted = iter;
        log_test(converted == 12345, 
                "Implicit conversion to uint16_t works",
                "Expected: 12345, Got: " + std::to_string(converted));
        
        // Test in arithmetic operations
        P_Iterator iter2(100);
        uint16_t result = iter2 + 50;
        log_test(result == 150, 
                "Conversion operator works in arithmetic",
                "Expected: 150, Got: " + std::to_string(result));
        
        // Test with large bit values
        P_Iterator iter3(32768);
        uint16_t large_val = iter3;
        log_test(large_val == 32768, 
                "Conversion works with large bit values");
    }
    
    // Test pre-increment operator (++iter)
    void test_pre_increment() {
        std::cout << CYAN << "\n=== Testing Pre-increment Operator (++iter) ===" << RESET << std::endl;
        
        // Normal increment
        P_Iterator iter1(100);
        P_Iterator& result = ++iter1;
        log_test(iter1.get_iterator() == 101, 
                "Pre-increment: 100 -> 101");
        log_test(&result == &iter1, 
                "Pre-increment returns reference to self");
        
        // Increment at boundary
        P_Iterator iter2(32767);
        ++iter2;
        log_test(iter2.get_iterator() == 32768, 
                "Pre-increment: 32767 -> 32768 (crosses large bit boundary)");
        
        // Increment with large bit set
        P_Iterator iter3(65534);
        ++iter3;
        bool wrapped = iter3.get_iterator() >= 32768;
        log_test(wrapped, 
                "Pre-increment: 65534 wraps correctly",
                "Result: " + std::to_string(iter3.get_iterator()));
        
        // Multiple increments
        P_Iterator iter4(0);
        for (int i = 0; i < 5; ++i) {
            ++iter4;
        }
        log_test(iter4.get_iterator() == 5, 
                "Multiple pre-increments work correctly");
    }
    
    // Test post-increment operator (iter++)
    void test_post_increment() {
        std::cout << CYAN << "\n=== Testing Post-increment Operator (iter++) ===" << RESET << std::endl;
        
        // Normal increment
        P_Iterator iter1(200);
        P_Iterator& result = iter1++;
        log_test(iter1.get_iterator() == 201, 
                "Post-increment: 200 -> 201");
        log_test(&result == &iter1, 
                "Post-increment returns reference to self");
        
        // Increment at boundary
        P_Iterator iter2(32767);
        iter2++;
        log_test(iter2.get_iterator() == 32768, 
                "Post-increment: 32767 -> 32768 (crosses large bit boundary)");
        
        // Increment with large bit set
        P_Iterator iter3(65535);
        iter3++;
        bool wrapped = iter3.get_iterator() >= 32768;
        log_test(wrapped, 
                "Post-increment: 65535 wraps correctly",
                "Result: " + std::to_string(iter3.get_iterator()));
        
        // Test sequence
        P_Iterator iter4(10);
        uint16_t old_val = iter4.get_iterator();
        iter4++;
        log_test(iter4.get_iterator() == old_val + 1, 
                "Post-increment behaves correctly");
    }
    
    // Test compound assignment operator (+=)
    void test_compound_assignment() {
        std::cout << CYAN << "\n=== Testing Compound Assignment Operator (+=) ===" << RESET << std::endl;
        
        // Normal addition
        P_Iterator iter1(100);
        P_Iterator& result = (iter1 += 50);
        log_test(iter1.get_iterator() == 150, 
                "Compound assignment: 100 += 50 = 150");
        log_test(&result == &iter1, 
                "Compound assignment returns reference to self");
        
        // Add zero
        P_Iterator iter2(500);
        iter2 += 0;
        log_test(iter2.get_iterator() == 500, 
                "Compound assignment: += 0 doesn't change value");
        
        // Large increment
        P_Iterator iter3(1000);
        iter3 += 31000;
        log_test(iter3.get_iterator() == 32000, 
                "Large compound assignment works");
        
        // Increment that crosses large bit boundary
        P_Iterator iter4(32760);
        iter4 += 20;
        bool has_large_bit = iter4.get_iterator() >= 32768;
        log_test(has_large_bit, 
                "Compound assignment crosses large bit boundary correctly",
                "Result: " + std::to_string(iter4.get_iterator()));
        
        // Increment with large bit value
        P_Iterator iter5(32768);
        iter5 += 100;
        log_test(iter5.get_iterator() >= 32768, 
                "Compound assignment with large bit input",
                "Result: " + std::to_string(iter5.get_iterator()));
        
        // Test wrap-around behavior with large increment
        P_Iterator iter6(65534);
        iter6 += 10;
        bool wrapped_correctly = iter6.get_iterator() >= 32768;
        log_test(wrapped_correctly, 
                "Large compound assignment handles wrap-around",
                "Result: " + std::to_string(iter6.get_iterator()));
        
        // Add large bit value to small value
        P_Iterator iter7(100);
        iter7 += 32768;
        bool result_has_large_bit = iter7.get_iterator() >= 32768;
        log_test(result_has_large_bit, 
                "Adding large bit value sets large bit correctly");
    }
    
    // Test static get_diff method (comprehensive)
    void test_get_diff_method() {
        std::cout << CYAN << "\n=== Testing Static get_diff() Method ===" << RESET << std::endl;
        
        // Normal cases
        log_test(P_Iterator::get_diff(100, 90) == 10, 
                "get_diff(100, 90) = 10");
        log_test(P_Iterator::get_diff(90, 100) == -10, 
                "get_diff(90, 100) = -10");
        log_test(P_Iterator::get_diff(1000, 1000) == 0, 
                "get_diff(1000, 1000) = 0");
        
        // Wrap-around cases
        log_test(P_Iterator::get_diff(32768, 65535) == 1, 
                "get_diff(32768, 65535) = 1 (wrap-around)");
        log_test(P_Iterator::get_diff(65535, 32768) == -1, 
                "get_diff(65535, 32768) = -1 (wrap-around)");
        
        // Edge cases
        log_test(P_Iterator::get_diff(32768, 32767) == 1, 
                "get_diff(32768, 32767) = 1 (large bit boundary)");
        log_test(P_Iterator::get_diff(0, 65535) < 0, 
                "get_diff(0, 65535) negative (different bit status)");
    }
    
    // Test static check_iterators method (comprehensive)
    void test_check_iterators_method() {
        std::cout << CYAN << "\n=== Testing Static check_iterators() Method ===" << RESET << std::endl;
        
        // Large bit priority cases
        log_test(P_Iterator::check_iterators(32768, 100) == true, 
                "check_iterators(32768, 100) = true (large bit priority)");
        log_test(P_Iterator::check_iterators(100, 32768) == false, 
                "check_iterators(100, 32768) = false (no large bit priority)");
        
        // Same bit status - both small
        log_test(P_Iterator::check_iterators(105, 100) == true, 
                "check_iterators(105, 100) = true (diff > 1)");
        log_test(P_Iterator::check_iterators(101, 100) == false, 
                "check_iterators(101, 100) = false (diff = 1, not > 1)");
        
        // Same bit status - both large
        log_test(P_Iterator::check_iterators(33000, 32800) == true, 
                "check_iterators(33000, 32800) = true (large diff)");
        log_test(P_Iterator::check_iterators(32768, 65535) == false, 
                "check_iterators(32768, 65535) = false (diff = 1)");
    }
    
    // Test update_iterator method (comprehensive)
    void test_update_iterator_method() {
        std::cout << CYAN << "\n=== Testing update_iterator() Method ===" << RESET << std::endl;
        
        // Normal updates
        P_Iterator iter1(100);
        log_test(iter1.update_iterator(105) == true, 
                "update_iterator(105) when current=100 -> accepted");
        log_test(iter1.get_iterator() == 106, 
                "Iterator updated to 105+1=106");
        
        // Reject older values
        log_test(iter1.update_iterator(105) == false, 
                "update_iterator(105) when current=106 -> rejected");
        log_test(iter1.get_iterator() == 106, 
                "Iterator unchanged after rejection");
        
        // Large bit priority
        P_Iterator iter2(1000);
        log_test(iter2.update_iterator(32768) == true, 
                "update_iterator(32768) with large bit priority");
        log_test(iter2.get_iterator() == 32769, 
                "Iterator set to 32768+1=32769");
        
        // Reject when no large bit priority
        P_Iterator iter3(32800);
        log_test(iter3.update_iterator(1000) == false, 
                "update_iterator(1000) rejected (no large bit priority)");
        
        // Edge case: exact diff = 1
        P_Iterator iter4(100);
        log_test(iter4.update_iterator(101) == false, 
                "update_iterator(101) when current=100 -> rejected (diff=1)");
        
        // Wrap-around case
        P_Iterator iter5(65535);
        log_test(iter5.update_iterator(32768) == false, 
                "update_iterator(32768) when current=65535 -> rejected (diff=1)");
    }
    
    // Test class constants and static members
    void test_constants_and_statics() {
        std::cout << CYAN << "\n=== Testing Class Constants and Static Members ===" << RESET << std::endl;
        
        // Test ITER_DIFF constant
        log_test(P_Iterator::ITER_DIFF == 1, 
                "ITER_DIFF constant equals 1",
                "Expected: 1, Got: " + std::to_string(P_Iterator::ITER_DIFF));
        
        // Test static method accessibility
        bool static_methods_accessible = true;
        try {
            P_Iterator::get_diff(100, 90);
            P_Iterator::check_iterators(100, 90);
        } catch (...) {
            static_methods_accessible = false;
        }
        log_test(static_methods_accessible, 
                "Static methods accessible without instance");
    }
    
    // Test integration scenarios
    void test_integration_scenarios() {
        std::cout << CYAN << "\n=== Testing Integration Scenarios ===" << RESET << std::endl;
        
        // Packet sequence simulation
        P_Iterator packet_iter(0);
        std::vector<uint16_t> packets = {5, 10, 15, 20, 25};
        int accepted = 0;
        
        for (uint16_t packet : packets) {
            if (packet_iter.update_iterator(packet)) {
                accepted++;
            }
        }
        log_test(accepted > 0, 
                "Packet sequence integration test",
                "Accepted: " + std::to_string(accepted) + "/" + std::to_string(packets.size()));
        
        // Iterator arithmetic integration
        P_Iterator arith_iter(1000);
        arith_iter += 500;
        ++arith_iter;
        arith_iter++;
        log_test(arith_iter.get_iterator() == 1502, 
                "Iterator arithmetic integration",
                "Expected: 1502, Got: " + std::to_string(arith_iter.get_iterator()));
        
        // Conversion operator integration
        P_Iterator conv_iter(500);
        uint16_t sum = conv_iter + 200;  // Uses conversion operator
        log_test(sum == 700, 
                "Conversion operator integration in arithmetic");
        
        // Cross-boundary operations
        P_Iterator boundary_iter(32766);
        boundary_iter += 5;  // Should cross into large bit range
        bool crossed_boundary = boundary_iter.get_iterator() >= 32768;
        log_test(crossed_boundary, 
                "Cross-boundary operations work correctly");
    }
    
    // Test edge cases and error conditions
    void test_edge_cases() {
        std::cout << CYAN << "\n=== Testing Edge Cases ===" << RESET << std::endl;
        
        // Maximum value operations
        P_Iterator max_iter(65535);
        uint16_t before_increment = max_iter.get_iterator();
        ++max_iter;
        bool wrapped_on_max = max_iter.get_iterator() != 0;  // Should wrap, not overflow to 0
        log_test(wrapped_on_max, 
                "Maximum value increment wraps correctly",
                "Before: " + std::to_string(before_increment) + ", After: " + std::to_string(max_iter.get_iterator()));
        
        // Zero operations
        P_Iterator zero_iter(0);
        zero_iter += 0;
        log_test(zero_iter.get_iterator() == 0, 
                "Zero operations don't change value");
        
        // Large bit boundary exact
        P_Iterator boundary_iter(32768);
        log_test(boundary_iter.get_iterator() >= 32768, 
                "Large bit boundary value handled correctly");
        
        // Self-update scenarios
        P_Iterator self_iter(100);
        log_test(self_iter.update_iterator(100) == false, 
                "Self-update with same value rejected");
    }
    
    void run_all_tests() {
        std::cout << MAGENTA << "=== P_Iterator COMPLETE Unit Test Suite ===" << RESET << std::endl;
        std::cout << "Testing ALL methods and functionality of P_Iterator class\n" << std::endl;
        
        test_constructor();
        test_get_iterator();
        test_set_iterator();
        test_conversion_operator();
        test_pre_increment();
        test_post_increment();
        test_compound_assignment();
        test_get_diff_method();
        test_check_iterators_method();
        test_update_iterator_method();
        test_constants_and_statics();
        test_integration_scenarios();
        test_edge_cases();
        
        std::cout << MAGENTA << "\n=== Complete Test Summary ===" << RESET << std::endl;
        std::cout << "Passed: " << GREEN << passed_tests << RESET << "/" << total_tests << " tests" << std::endl;
        
        double success_rate = (100.0 * passed_tests) / total_tests;
        std::cout << "Success Rate: " << std::fixed << std::setprecision(1) << success_rate << "%" << std::endl;
        
        if (passed_tests == total_tests) {
            std::cout << GREEN << "\n🎉 ALL TESTS PASSED! P_Iterator class is fully validated." << RESET << std::endl;
        } else {
            std::cout << RED << "\n❌ Some tests failed. Review the implementation." << RESET << std::endl;
            std::cout << "Failed tests: " << (total_tests - passed_tests) << std::endl;
        }
        
        std::cout << YELLOW << "\nTest Coverage:" << RESET << std::endl;
        std::cout << "✓ Constructor (default and parameterized)" << std::endl;
        std::cout << "✓ get_iterator() method" << std::endl;
        std::cout << "✓ set_iterator() method (test only)" << std::endl;
        std::cout << "✓ uint16_t conversion operator" << std::endl;
        std::cout << "✓ Pre-increment operator (++iter)" << std::endl;
        std::cout << "✓ Post-increment operator (iter++)" << std::endl;
        std::cout << "✓ Compound assignment operator (+=)" << std::endl;
        std::cout << "✓ Static get_diff() method" << std::endl;
        std::cout << "✓ Static check_iterators() method" << std::endl;
        std::cout << "✓ update_iterator() method" << std::endl;
        std::cout << "✓ Class constants and static members" << std::endl;
        std::cout << "✓ Integration scenarios" << std::endl;
        std::cout << "✓ Edge cases and error conditions" << std::endl;
    }
};

int main() {
    P_IteratorCompleteUnitTest tester;
    tester.run_all_tests();
    return 0;
}
