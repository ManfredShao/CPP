# MSYS2 MinGW64 版 Makefile
CC = gcc
CFLAGS = -std=gnu11 -D_ISOC11_SOURCE -g -O3 -march=native -fopenmp -Wall \
         -I/mingw64/include/openblas         
LDFLAGS = -fopenmp -L/mingw64/lib -lopenblas

TARGET = sgemm_bench

SRCS := main.c interface.c sgemm.c
OBJS := $(SRCS:.c=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) 2000

clean:
	rm -f *.o $(TARGET)