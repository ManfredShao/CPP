BUILD_DIR ?= build

.PHONY: all release run clean

all: release

release:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DUSE_OPENBLAS=ON
	cmake --build $(BUILD_DIR) -j

run: release
	./$(BUILD_DIR)/project3 --sizes 16,128,1024 --runs 3 --plain-max 1024 --mem-gib 8 --csv result.csv

clean:
	rm -rf $(BUILD_DIR) result.csv