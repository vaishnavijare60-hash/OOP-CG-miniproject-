CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -O2 -Iinclude

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lgdi32 -luser32
    TARGET = banking_system.exe
    MKDIR_CMD = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
    RM_CMD = if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR) && if exist $(TARGET) del /f /q $(TARGET)
else
    LDFLAGS =
    TARGET = banking_system
    MKDIR_CMD = mkdir -p $(OBJ_DIR)
    RM_CMD = rm -rf $(OBJ_DIR) $(TARGET)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR_CMD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM_CMD)

.PHONY: all clean
