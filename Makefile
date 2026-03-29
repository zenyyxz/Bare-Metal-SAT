CC=gcc
CXX=g++
CFLAGS=-O3 -Wall
CXXFLAGS=-O3 -Wall -std=c++17
AS=nasm
ASFLAGS=-f elf64

SRC_DIR=src
INC_DIR=include
OBJ_DIR=obj

SRCS=$(SRC_DIR)/sat_solver.cpp $(SRC_DIR)/low_level.asm $(SRC_DIR)/bitset.c
OBJS=$(OBJ_DIR)/sat_solver.o $(OBJ_DIR)/low_level.o $(OBJ_DIR)/bitset.o

TARGET=sat_solver

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: clean
