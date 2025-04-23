BISON = bison
CXX = g++
CXXFLAGS = -std=c++20

all: qux

qux.cc: qux.cc.re
	re2c -o "$@" "$<"

qux.cc.re: src/qux.y
	$(BISON) $(BISONFLAGS) -o $@ $<

qux: qux.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

