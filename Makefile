CXX      := g++ 
CXXFLAGS := -std=c++11 -O3
LDFLAGS  := 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/apps
SHELL	 := /bin/bash

TARGET   := src

INCLUDE  := -I$(TARGET)/Camera/ -I$(TARGET)/Image -I$(TARGET)/Light -I$(TARGET)/Primitive -I$(TARGET)/Primitive/BRDF -I$(TARGET)/Primitive/Geometry -I$(TARGET)/Rays -I$(TARGET)/Renderer -I$(TARGET)/Scene -I$(TARGET)/Shader -I$(TARGET)/utils -I$(TARGET)/Image/ToneMapper -I$(TARGET)/Image/PostFilter

SRC      :=                      \
	$(wildcard $(TARGET)/*.cpp) \
	$(wildcard $(TARGET)/Camera/*.cpp)         \
	$(wildcard $(TARGET)/Image/*.cpp)         \
	$(wildcard $(TARGET)/Primitive/BRDF/*.cpp)         \
	$(wildcard $(TARGET)/Primitive/Geometry/*.cpp)         \
	$(wildcard $(TARGET)/Renderer/*.cpp)         \
	$(wildcard $(TARGET)/Scene/*.cpp)         \
	$(wildcard $(TARGET)/Shader/*.cpp)         \

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPENDENCIES \
		:= $(OBJECTS:.o=.d)

# --- Módulo Mitsuba: 2º binário (driver de renderização separado da main) ---
# Reutiliza todos os objetos do motor EXCETO o main.o, + o vi_rt_render.o próprio.
MITSUBA_SRC     := $(TARGET)/mitsuba/vi_rt_render.cpp $(TARGET)/mitsuba/MitsubaExporter.cpp
MITSUBA_OBJ     := $(MITSUBA_SRC:%.cpp=$(OBJ_DIR)/%.o)
ENGINE_OBJECTS  := $(filter-out $(OBJ_DIR)/$(TARGET)/main.o,$(OBJECTS))

all:	build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	cp $(TARGET)/Image/*.ppm $(APP_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

# Binário do módulo Mitsuba (driver de renderização das cenas de referência)
mitsuba: build $(APP_DIR)/mitsuba_render

$(APP_DIR)/mitsuba_render: $(ENGINE_OBJECTS) $(MITSUBA_OBJ)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/mitsuba_render $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean mitsuba

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

run: $(APP_DIR)/$(TARGET)
	@echo "Running $(TARGET)..."
	@cd $(APP_DIR) && ./$(TARGET)
	@$(MAKE) display

display:
	@echo "Displaying output image..."
	@if [[ "$$(uname)" == "Darwin" ]]; then open $(APP_DIR)/result/*.ppm; else display -resize 1280x1280 $(APP_DIR)/result/*.ppm; fi
