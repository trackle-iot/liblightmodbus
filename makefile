all:
	clang -o demo demo.c -Wall -Wextra -fsanitize=address -fsanitize=alignment -Iinclude -Wno-unused-parameter
