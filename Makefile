CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
LDFLAGS = -lgpiodcxx

TARGET = alarm_clock
SRCS = src/main.cpp src/buttons.cpp src/display.cpp src/modes.cpp src/buzzer.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)