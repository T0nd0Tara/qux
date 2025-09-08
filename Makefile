BISON=bison
CXX=g++
CXXFLAGS = -std=c++20 -I/home/amirs/code-libs/magic_enum/include -DYYDEBUG=1 -g -ggdb
BUILD_DIR=build

all: $(BUILD_DIR)/qux

test: $(BUILD_DIR)/qux
	deno run --allow-read test/main.ts

clean:
	rm -f $(BUILD_DIR)/qux.cc $(BUILD_DIR)/qux.re.cc $(BUILD_DIR)/qux

$(BUILD_DIR)/qux.cc: $(BUILD_DIR)/qux.re.cc
	re2c -o $@ $<

$(BUILD_DIR)/qux.re.cc: src/qux.y
	$(BISON) $(BISONFLAGS) -o $@ $<

$(BUILD_DIR)/qux: $(BUILD_DIR)/qux.cc
	$(CXX) $(CXXFLAGS) -o $@ $<
