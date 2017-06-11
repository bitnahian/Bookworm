CC = clang
CFLAGS = -g -O0 -Wall -Werror -std=gnu11 -march=native -lpthread -fsanitize=address

.PHONY: all clean

all: worm

worm: main.c worm.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	-rm -f *.o
	-rm -f worm
	-rm -rf *.dSYM
