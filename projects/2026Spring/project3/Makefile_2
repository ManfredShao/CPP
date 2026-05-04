CC := $(or $(shell command -v gcc-14 2>/dev/null),$(shell command -v gcc-13 2>/dev/null),$(shell command -v gcc-12 2>/dev/null),gcc)
CC_VERSION := $(shell $(CC) --version 2>/dev/null | head -n 1)

TARGET := sgemm_bench
SRCS := main.c interface.c sgemm.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

OPT_FLAGS ?= -g -O3 -march=native -ffast-math
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
MACOSX_SDK_VERSION := $(shell /usr/bin/xcrun --sdk macosx --show-sdk-version 2>/dev/null)
ifeq ($(strip $(MACOSX_SDK_VERSION)),)
MACOSX_DEPLOYMENT_TARGET ?= 16.0
else
MACOSX_DEPLOYMENT_TARGET ?= $(MACOSX_SDK_VERSION)
endif
export MACOSX_DEPLOYMENT_TARGET
endif

ifeq ($(findstring Apple clang,$(CC_VERSION)),Apple clang)
COMPILER_FAMILY := apple-clang
else ifneq ($(findstring clang,$(CC_VERSION)),)
COMPILER_FAMILY := clang
else
COMPILER_FAMILY := gcc
endif

# Optional ISA tuning (example: ARCH_FLAGS=-mavx2)
ARCH_FLAGS ?=

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
ARCH_FLAGS :=
endif
DEPFLAGS ?= -MMD -MP

ifeq ($(COMPILER_FAMILY),apple-clang)
WARN_FLAGS ?= -Wall -Wextra -Wno-overriding-deployment-version
else ifeq ($(COMPILER_FAMILY),clang)
WARN_FLAGS ?= -Wall -Wextra -Wpedantic -Wno-overriding-deployment-version
else
WARN_FLAGS ?= -Wall -Wextra -Wpedantic
endif

LIBOMP_INCLUDE_DIR ?= /opt/homebrew/opt/libomp/include
LIBOMP_LIB_DIR ?= /opt/homebrew/opt/libomp/lib

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

CFLAGS ?= $(OPT_FLAGS) $(ARCH_FLAGS) $(OMP_CFLAGS) $(WARN_FLAGS) $(DEPFLAGS)

OPENBLAS_CFLAGS ?= $(shell pkg-config --cflags openblas 2>/dev/null)
OPENBLAS_LIBS ?= $(shell pkg-config --libs openblas 2>/dev/null)

ifeq ($(strip $(OPENBLAS_LIBS)),)
OPENBLAS_PREFIX ?= /opt/homebrew/opt/openblas
CPPFLAGS += -I$(OPENBLAS_PREFIX)/include/openblas -I$(OPENBLAS_PREFIX)/include
LDFLAGS += -L$(OPENBLAS_PREFIX)/lib $(OMP_LDFLAGS)
LDLIBS += -lopenblas $(OMP_LIBS)
else
CPPFLAGS += $(OPENBLAS_CFLAGS)
LDFLAGS += $(OMP_LDFLAGS)
LDLIBS += $(OPENBLAS_LIBS) $(OMP_LIBS)
endif

.PHONY: all run clean info

RUN_SIZE ?= 1000

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET) $(RUN_SIZE)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

info:
	@echo "CC=$(CC)"
	@echo "CC_VERSION=$(CC_VERSION)"
	@echo "COMPILER_FAMILY=$(COMPILER_FAMILY)"
	@echo "CFLAGS=$(CFLAGS)"
	@echo "CPPFLAGS=$(CPPFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "LDLIBS=$(LDLIBS)"
	@echo "OPENBLAS_CFLAGS=$(OPENBLAS_CFLAGS)"
	@echo "OPENBLAS_LIBS=$(OPENBLAS_LIBS)"
