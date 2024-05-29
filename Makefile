SRCS := main.cpp info_manager.cpp sample_manager.cpp instrument_manager.cpp preset_manager.cpp sfmap.cpp sfzone.cpp
OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DEPS := $(patsubst %.cpp,%.d,$(SRCS))

.PHONY: clean

CXXFLAGS := -O0 -ggdb3 -Wall -std=c++20

sftest: $(OBJS)
	g++ $(CXXFLAGS) $^ -o $@

-include ($(DEPS))

%.o: %.cpp Makefile
	g++ $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm $(OBJS) $(DEPS)
