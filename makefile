CFLAGS = -O3 -Wall

.PHONY: all clean tests

all: shared 
shared: hashmap.h hashmap.c
	$(CC) -fPIC -O3 -c hashmap.c
	$(CC) -shared -Wl,--export-dynamic hashmap.o -o libhashmap.so

clean:
	rm -rf example *.o *.so
