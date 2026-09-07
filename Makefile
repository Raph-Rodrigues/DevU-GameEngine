# Wrapper de conveniência em cima do CMake.
# Quem realmente compila o projeto é o CMake (é ele quem sabe achar o SDL3
# via find_package); este Makefile só evita ter que digitar
# "cmake -S . -B build && cmake --build build" toda hora.

BUILD_DIR  := build
TARGET     := the-hunter
BIN        := $(BUILD_DIR)/bin/$(TARGET)
BUILD_TYPE ?= Debug

.PHONY: all configure build run clean rebuild

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) -j

run: build
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build
