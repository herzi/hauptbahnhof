all: demo

sources=\
	main.c \
	queue.c \
	worker.c \
	$(NULL)
headers=\
	queue.h \
	worker.h \
	$(NULL)

demo: $(sources:.c=.o) Makefile
	gcc -ggdb -o $@ $(sources:.c=.o) $(shell pkg-config --libs gthread-2.0)

%.o: %.c $(headers) Makefile
	gcc -ggdb -c -o $@ $< $(shell pkg-config --cflags gthread-2.0)

clean:
	rm -rf *.o
	rm -rf demo
