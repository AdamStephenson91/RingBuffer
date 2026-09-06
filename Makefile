CXX       := g++
CXXFLAGS  := -Wall -Wextra -std=c++23 -O3 -DNDEBUG -g -pthread -I. -I./ParserOutput -MMD -MP

LDFLAGS   := -L/usr/local/lib -pthread -lbenchmark

TARGET    := RingBufferDemo

SRCS      := $(wildcard *.cc)
OBJS      := $(SRCS:.cc=.o)
DEPS      := $(SRCS:.cc=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)

.PHONY: all clean
