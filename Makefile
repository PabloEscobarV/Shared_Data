CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG_SSV -DDEBUG_SSRV -DDEBUG_SSE
SOURCES = src/p_iterator.cpp src/shared_param.cpp

# Default target
all: test

# Test programs
test: test.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_enhanced: test_enhanced.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_minimal: test_minimal.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_silent: test_silent.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# --- Add the new test target below ---

# Unit test for P_Iterator class
test_iterator: test_p_iterator.cpp src/p_iterator.cpp
	@echo "Compiling P_Iterator unit tests..."
	$(CXX) $(CXXFLAGS) -Ihdrs test_p_iterator.cpp src/p_iterator.cpp -o test_p_iterator
	@echo "Running P_Iterator unit tests..."
	./test_p_iterator

# Comprehensive unit test for P_Iterator class
test_p_iterator_complete: test_p_iterator_complete.cpp src/p_iterator.cpp
	@echo "Compiling comprehensive P_Iterator unit tests..."
	$(CXX) $(CXXFLAGS) test_p_iterator_complete.cpp src/p_iterator.cpp -o test_p_iterator_complete
	@echo "Running comprehensive P_Iterator unit tests..."
	./test_p_iterator_complete

# Advanced unit test for P_Iterator (update_iterator and wrap-around focus)
test_p_iterator_advanced: test_p_iterator_advanced.cpp src/p_iterator.cpp
	@echo "Compiling advanced P_Iterator unit tests..."
	$(CXX) $(CXXFLAGS) test_p_iterator_advanced.cpp src/p_iterator.cpp -o test_p_iterator_advanced
	@echo "Running advanced P_Iterator unit tests..."
	./test_p_iterator_advanced

# Comprehensive test for check_iterators function
test_check_iterators: test_check_iterators.cpp src/p_iterator.cpp
	@echo "Compiling check_iterators function tests..."
	$(CXX) $(CXXFLAGS) test_check_iterators.cpp src/p_iterator.cpp -o test_check_iterators
	@echo "Running check_iterators function tests..."
	./test_check_iterators

# Final comprehensive P_Iterator unit tests with wrap-around
test_p_iterator_final: test_p_iterator_final.cpp src/p_iterator.cpp
	@echo "Compiling final P_Iterator unit tests..."
	$(CXX) $(CXXFLAGS) test_p_iterator_final.cpp src/p_iterator.cpp -o test_p_iterator_final
	@echo "Running final P_Iterator unit tests..."
	./test_p_iterator_final

# Ultimate comprehensive unit test for P_Iterator (ALL functionality)
test_p_iterator_ultimate: test_p_iterator_ultimate.cpp src/p_iterator.cpp
	@echo "Compiling ULTIMATE P_Iterator comprehensive tests..."
	$(CXX) $(CXXFLAGS) test_p_iterator_ultimate.cpp src/p_iterator.cpp -o test_p_iterator_ultimate
	@echo "Running ULTIMATE P_Iterator comprehensive tests..."
	./test_p_iterator_ultimate

test_debug: test_debug.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Full algorithm test
test_full_algorithm: test_full_algorithm.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# SSE working test
test_sse_working: test_sse_working.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Ultimate comprehensive test
test_ultimate_100_final: test_ultimate_100_final.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Run targets
run: test
	./test

# Clean targets
clean:
	rm -f test test_enhanced test_minimal test_silent test_debug test_full_algorithm test_sse_working test_ultimate_100_final test_p_iterator *.log
