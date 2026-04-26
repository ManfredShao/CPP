CC := $(or $(shell command -v gcc-14 2>/dev/null),$(shell command -v gcc-13 2>/dev/null),$(shell command -v gcc-12 2>/dev/null),gcc)
CC_VERSION := $(shell $(CC) --version 2>/dev/null | head -n 1)

TARGET := sgemm_bench
SRCS := main.c interface.c sgemm.c
OBJS := $(SRCS:.c=.o)

OPT_FLAGS ?= -O3 -march=native

ifeq ($(findstring Apple clang,$(CC_VERSION)),Apple clang)
COMPILER_FAMILY := apple-clang
else ifneq ($(findstring clang,$(CC_VERSION)),)
COMPILER_FAMILY := clang
else
COMPILER_FAMILY := gcc
endif

ARCH_FLAGS ?= -mavx2

ifeq ($(COMPILER_FAMILY),apple-clang)
WARN_FLAGS ?= -Wall -Wextra
else ifeq ($(COMPILER_FAMILY),clang)
WARN_FLAGS ?= -Wall -Wextra -Wpedantic
else
WARN_FLAGS ?= -Wall -Wextra -Wpedantic
endif

LIBOMP_INCLUDE_DIR ?= /opt/local/include/libomp
LIBOMP_LIB_DIR ?= /opt/local/lib/libomp

ifeq ($(COMPILER_FAMILY),apple-clang)
OMP_CFLAGS ?= -Xpreprocessor -fopenmp -I$(LIBOMP_INCLUDE_DIR)
OMP_LDFLAGS ?= -L$(LIBOMP_LIB_DIR) -Wl,-rpath,$(LIBOMP_LIB_DIR)
OMP_LIBS ?= -lomp
else ifeq ($(COMPILER_FAMILY),clang)
OMP_CFLAGS ?= -fopenmp
OMP_LDFLAGS ?=
OMP_LIBS ?= -fopenmp
else
OMP_CFLAGS ?= -fopenmp
OMP_LDFLAGS ?=
OMP_LIBS ?= -fopenmp
endif

CFLAGS ?= $(OPT_FLAGS) $(ARCH_FLAGS) $(OMP_CFLAGS) $(WARN_FLAGS)

OPENBLAS_PREFIX ?= /opt/local
CPPFLAGS += -I$(OPENBLAS_PREFIX)/include/openblas -I$(OPENBLAS_PREFIX)/include
LDFLAGS += -L$(OPENBLAS_PREFIX)/lib $(OMP_LDFLAGS)
LDLIBS += -lopenblas $(OMP_LIBS)

.PHONY: all run clean info

RUN_SIZE ?= 1000

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(RUN_SIZE)

clean:
	rm -f $(OBJS) $(TARGET)

info:
	@echo "CC=$(CC)"
	@echo "CC_VERSION=$(CC_VERSION)"
	@echo "COMPILER_FAMILY=$(COMPILER_FAMILY)"
	@echo "CFLAGS=$(CFLAGS)"
	@echo "CPPFLAGS=$(CPPFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "LDLIBS=$(LDLIBS)"
