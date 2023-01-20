CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

all: httpserver

httpserver: httpserver.o
	$(CC) -o httpserver httpserver.o
httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c
clean:
	rm -f httpserver httpserver.o bind.o
scan_build: clean
	scan-build make
