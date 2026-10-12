CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
ASAN_OPTIONS ?= detect_leaks=1
SOURCES = $(wildcard src/*.cpp)
CORE = $(filter-out src/main.cpp,$(SOURCES))
OBJECTS = $(patsubst src/%.cpp,build/%.o,$(SOURCES))

all: bin/dj_manager

bin/dj_manager: $(OBJECTS) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build bin:
	mkdir -p $@

build/tests: tests/tests.cpp $(CORE) $(wildcard include/*.h) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/tests.cpp $(CORE) -o $@

test: build/tests
	./build/tests

sanitize: | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tests/tests.cpp $(CORE) -o build/tests-sanitize
	ASAN_OPTIONS=$(ASAN_OPTIONS) ./build/tests-sanitize

demo: all
	./bin/dj_manager journey examples/night_drive.txt night-drive report.html

clean:
	rm -rf build bin/dj_manager

-include $(OBJECTS:.o=.d)
.PHONY: all test sanitize demo clean
