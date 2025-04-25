BISON = bison
CXX = g++
CXXFLAGS = -std=c++20

all: qux

test: qux
	deno run --allow-read test/main.ts

clean:
	rm -f qux.cc qux.re.cc qux

qux.cc: qux.re.cc
	re2c -o $@ $<

qux.re.cc: src/qux.y
	$(BISON) $(BISONFLAGS) -o $@ $<

qux: qux.cc
	$(CXX) $(CXXFLAGS) -o $@ $<
