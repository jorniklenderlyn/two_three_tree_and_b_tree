CXX = g++
#-DENABLE_LOGGING
CXXFLAGS = -DENABLE_LOGGING -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
DEPFLAGS = -MMD -MP

BUILD_DIR = build

SOURCES = $(wildcard *.cpp)

OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/app.exe

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

.PHONY: clean
clean:
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)

.PHONY: rebuild
rebuild: clean all

.PHONY: run
run: $(TARGET)
	$(TARGET)

.PHONY: info
info:
	@echo Sources: $(SOURCES)
	@echo Objects: $(OBJECTS)
	@echo Target: $(TARGET)

-include $(DEPS)