CC = gcc
CFLAGS = -O3 -pthread
DEPS = thread
OUT = test.out
all:
	$(CC) $(CFLAGS) $(addsuffix .c, $(DEPS)) -o $(OUT)

clean:
	rm -f $(OUT) *.o