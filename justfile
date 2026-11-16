default: run

CC := "gcc"
CC_FLAGS := "-Wall -Wextra"
BUILD_FOLDER := `realpath ./build`
RELEASE_BIN_FOLDER := BUILD_FOLDER + "/bin"
RELEASE_OBJ_FOLDER := BUILD_FOLDER + "/obj"
RELEASE_TARGET := RELEASE_BIN_FOLDER + "/server"

compile src target:
  {{ CC }} {{ CC_FLAGS }} -c {{ src }} -o {{ RELEASE_OBJ_FOLDER }}/{{ target }}

build-setup:
  @mkdir -p {{ RELEASE_BIN_FOLDER }} {{ RELEASE_OBJ_FOLDER }}

build-objs: build-setup
  just compile src/main.c main.o
  just compile src/http.c http.o
  just compile src/server.c server.o
  just compile src/router.c router.o

build: build-objs
  {{ CC }} {{ CC_FLAGS }} {{ RELEASE_OBJ_FOLDER }}/*.o -o {{ RELEASE_TARGET }}
  @echo
  @echo "\033[1;30;42m Success \033[0m Binary built to" {{ RELEASE_TARGET }}
  @echo

run: build
  {{ RELEASE_TARGET }}

clean:
  rm -rf {{ BUILD_FOLDER }}
