# GBS Shell Makefile

CC = g++
CFLAGS  = -Wall -g -std=c++17
OBJ = gbsh.o

all: gbsh

gbsh: $(OBJ)
	$(CC) $(CFLAGS) -o gbsh $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<
