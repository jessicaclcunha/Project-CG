CXX      := g++
CXXFLAGS := -std=c++11 -O3
LDFLAGS  := 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
SHELL    := /bin/bash

TARGET   := RMSE
EXEC := rmse_exec


INCLUDE  := -I$(TARGET)/Image -I$(TARGET)/utils -I$(TARGET)/Image/ToneMapper -I$(TARGET)/Image/PostFilter

SRC      :=                      \
	$(wildcard $(TARGET)/*.cpp) \
	$(wildcard $(TARGET)/Image/*.cpp)

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

all: build $(EXEC)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean run distance

build:
	@mkdir -p $(OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -f $(EXEC)