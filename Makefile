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
	rm -f test test_enhanced test_minimal test_silent test_debug test_full_algorithm test_sse_working test_ultimate_100_final *.log
