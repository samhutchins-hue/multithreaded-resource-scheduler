UNAME_S := $(shell uname -s)

# Compiler selection
ifeq ($(UNAME_S),Darwin)
  CXX = clang++
else
  CXX = g++
endif

# Common flags
CXXFLAGS = -std=c++17 -O2 -g -Wall -pthread -fno-omit-frame-pointer

# Linux-only flags (macOS will choke on -no-pie)
ifeq ($(UNAME_S),Linux)
  CXXFLAGS += -no-pie
endif

SRCS = main.cpp Storehouse.cpp Worker.cpp Auditor.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = simulation

all: $(TARGET)
	@echo "Build complete."
	@rm -f $(OBJS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

clean_objs:
	rm -f $(OBJS)

.PHONY: all clean clean_objs
