PROJECT_NAME := teamwork
BUILD_DIR := build
CMAKE := cmake
CTEST := ctest

.PHONY: all configure build test run clean distclean

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR)

build: configure
	$(CMAKE) --build $(BUILD_DIR)

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./bin/$(PROJECT_NAME)

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean

distclean:
	rm -rf $(BUILD_DIR)
