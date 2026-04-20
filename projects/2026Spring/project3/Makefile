CC := gcc

TARGET := sgemm_bench
SRCS := main.c interface.c time.c sgemm.c
OBJS := $(SRCS:.c=.o)

CFLAGS ?= -O3 -Wall -Wextra
CPPFLAGS += -I/opt/local/include/openblas
LDFLAGS += -L/opt/local/lib
LDLIBS += -lopenblas -lpthread

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) 1000

clean:
	rm -f $(OBJS) $(TARGET)
