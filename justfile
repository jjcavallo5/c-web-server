default: run

CC := "gcc"
CC_FLAGS := "-Wall -Wextra"
BUILD_FOLDER := `realpath ./build`

RELEASE_BIN_FOLDER := BUILD_FOLDER + "/release/bin"
RELEASE_OBJ_FOLDER := BUILD_FOLDER + "/release/obj"
RELEASE_TARGET := RELEASE_BIN_FOLDER + "/server"

TEST_BIN_FOLDER := BUILD_FOLDER + "/test/bin"
TEST_OBJ_FOLDER := BUILD_FOLDER + "/test/obj"
TEST_BIN_TARGET := TEST_BIN_FOLDER + "/run-test"

compile mode src target:
  {{ CC }} {{ CC_FLAGS }} -c {{ src }} -o {{ if mode == "release" { RELEASE_OBJ_FOLDER } else { TEST_OBJ_FOLDER } }}/{{ target }}

build-setup:
  @mkdir -p {{RELEASE_BIN_FOLDER}} {{TEST_BIN_FOLDER}}
  @mkdir -p {{RELEASE_OBJ_FOLDER}} {{TEST_OBJ_FOLDER}}

build-objs mode: build-setup
  just compile {{ mode }} src/http.c http.o
  just compile {{ mode }} src/server.c server.o
  just compile {{ mode }} src/router.c router.o

build:
  just build-objs release
  just compile release src/main.c main.o

  {{ CC }} {{ CC_FLAGS }} {{ RELEASE_OBJ_FOLDER }}/*.o -o {{ RELEASE_TARGET }}

  @echo
  @echo "\033[1;30;42m Success \033[0m Binary built to" {{ RELEASE_TARGET }}
  @echo

run: build
  {{ RELEASE_TARGET }}

build-test:
  just build-objs test
  just compile test tests/router-test.c router-test.o

  {{ CC }} {{ CC_FLAGS }} {{ TEST_OBJ_FOLDER }}/*.o -o {{ TEST_BIN_TARGET }}

  @echo
  @echo "\033[1;30;42m Success \033[0m Binary built to" {{ RELEASE_TARGET }}
  @echo

test: build-test
  {{ TEST_BIN_TARGET }}

clean:
  rm -rf {{ BUILD_FOLDER }}
