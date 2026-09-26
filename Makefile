BISON=bison
CXX=clang++
CXXFLAGS= -std=c++20 -I./magic_enum/include -I. -DYYDEBUG=1 -Wswitch-enum -Wswitch -g -ggdb
BOOSTFLAGS=-lboost_program_options
BUILD_DIR=bin
SRC_DIR=src

all: $(BUILD_DIR)/qux

test: $(BUILD_DIR)/qux
	bun test
.PHONY: test

clean:
	rm -f $(BUILD_DIR)/*

$(BUILD_DIR)/lexer.o: $(SRC_DIR)/lexer.cpp $(SRC_DIR)/lexer.hpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(BUILD_DIR)/ast.o: $(SRC_DIR)/ast.cpp $(SRC_DIR)/ast.hpp $(SRC_DIR)/lexer.hpp $(BUILD_DIR)/typing.o
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(BUILD_DIR)/typing.o: $(SRC_DIR)/typing.cpp $(SRC_DIR)/typing.hpp $(SRC_DIR)/ast.hpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(BUILD_DIR)/ir.o: $(SRC_DIR)/ir.cpp $(SRC_DIR)/ir.hpp $(SRC_DIR)/ast.hpp $(SRC_DIR)/typing.hpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(BUILD_DIR)/qux: $(SRC_DIR)/qux.cpp $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/typing.o $(BUILD_DIR)/ir.o
	$(CXX) $(CXXFLAGS) $(BOOSTFLAGS) -o $@ $^ 
