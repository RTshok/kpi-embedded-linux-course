CC = gcc
CFLAGS = -O3 -pthread
DEPS = thread
OUT = test
all:
	$(CC) $(CFLAGS) $(addsuffix .c, $(DEPS)) -o test

clean:
	rm -f test *.o