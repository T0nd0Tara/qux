BISON=bison
CXX=clang++
CXXFLAGS= -std=c++20 -I./magic_enum/include -I. -DYYDEBUG=1 -g -ggdb
BOOSTFLAGS=-lboost_program_options
BUILD_DIR=build
SRC_DIR=src

all: $(BUILD_DIR)/qux

test: $(BUILD_DIR)/qux
	deno run --allow-read --allow-write --allow-run test/main.ts
.PHONY: test

clean:
	rm -f $(BUILD_DIR)/*

$(BUILD_DIR)/qux.cc: $(BUILD_DIR)/qux.re.cc
	re2c -o $@ $<

$(BUILD_DIR)/qux.re.cc: $(SRC_DIR)/qux.y
	$(BISON) $(BISONFLAGS) -o $@ $<

$(BUILD_DIR)/qux: $(BUILD_DIR)/qux.cc $(SRC_DIR)/fill_typing.hpp $(SRC_DIR)/types.hpp $(SRC_DIR)/ir.hpp $(SRC_DIR)/macros.hpp $(SRC_DIR)/debugging.hpp
	$(CXX) $(CXXFLAGS) $(BOOSTFLAGS) -o $@ $< 
