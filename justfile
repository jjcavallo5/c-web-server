default: build

release_binary_folder := "build/bin"
release_intermediate_folder := "build/obj"
release_bin_path := `realpath build/bin/server`

build:
  mkdir -p {{ release_binary_folder }} {{ release_intermediate_folder }}
  gcc -Wall -Wextra -o {{ release_bin_path }} src/main.c
  @echo
  @echo "\033[1;30;42m Success \033[0m Binary built to" {{ release_bin_path }}
  @echo
