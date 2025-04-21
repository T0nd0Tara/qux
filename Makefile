BISON = bison
CXX = g++
CXXFLAGS = -std=c++20

all: qux

qux.cc: src/qux.y
	$(BISON) $(BISONFLAGS) -o $*.cc $<

qux: qux.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

