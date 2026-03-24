CXX ?= g++
CXXFLAGS += -Wall -Wextra -std=c++14
TARGET = daemon

all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(LDFLAGS) -o $@ $^

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o
