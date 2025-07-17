CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG_SSV -DDEBUG_SSRV -DDEBUG_SSE
SOURCES = src/p_iterator.cpp src/shared_param.cpp

# Default target
all: test_full_algorithm

# Test programs
test: test.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_enhanced: test_enhanced.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_minimal: test_minimal.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_silent: test_silent.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_debug: test_debug.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Full algorithm test (NEW)
test_full_algorithm: test_full_algorithm.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Debug version
test_full_algorithm_debug: test_full_algorithm.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -DDEBUG -o $@ $^

# Run targets
run: test_full_algorithm
	./test_full_algorithm 100 2 45000

run_debug: test_full_algorithm_debug
	./test_full_algorithm_debug 100 2 30000

run_quick: test_full_algorithm
	./test_full_algorithm 50 1 20000

# Test queue functionality
test_queue: test_queue_swap_fixed.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Ultimate comprehensive test
test_ultimate_100_final: test_ultimate_100_final.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# SSE working test
test_sse_working: test_sse_working.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Improved SSE test
test_improved_sse: test_improved_sse.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ $^

# Clean targets
clean:
	rm -f test test_enhanced test_minimal test_silent test_debug test_full_algorithm test_full_algorithm_debug test_queue test_ultimate_100_final test_sse_working test_improved_sse

.PHONY: all run run_debug run_quick clean
