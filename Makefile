NAME = shared_data
TEST = tests/test_proc_instance.cpp
# INFO_TEST = tests/test_info.cpp
# RUN_TEST = tests/run_test.cpp
# SSRV_TEST = tests/run_test.cpp

LIB_NAME = lib$(NAME).a
TEST_NAME = test
# INFO_TEST = info_test
# RUN_TEST = run_test
# SSRV_TEST = ssrv_test

OBJ_DIR = objs
OBJ_DIR_TEST = objs_test

SRC = p_iterator/p_iterator.cpp shared_param/shared_param.cpp fixed_size_queue/fixed_size_queue.cpp
SRC_TEST = $(wildcard tests/*.cpp)

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))
OBJ_TEST = $(addprefix $(OBJ_DIR_TEST)/, $(SRC_TEST:.cpp=.o))

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -static

.PHONY: all, cleam, fclean, test, info_test, ssrv_test, run_test, re

all: lib

lib: $(LIB_NAME)

clean:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_TEST)

fclean: clean
	rm -f $(NAME) $(LIB_NAME)

re: fclean
	lib

test: lib
	$(CXX) $(CXXFLAGS) $(LIB_NAME) $(TEST) -o $(TEST_NAME)
	./$(TEST_NAME)

$(LIB_NAME): $(OBJ)
	ar rcs $@ $(OBJ)

$(OBJ_DIR)/%.o = $(SRC)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) -c $< -o $@
	