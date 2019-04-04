CFLAGS += -I../

.PHONY: all

all: test

test: main.o strato3.o
	gcc -o test main.o strato3.o

main.o: main.c
	gcc $(CFLAGS) -c $< -o $@

strato3.o: ../strato3.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm *.o test
