all: demo

sources=\
	main.c \
	worker.c \
	$(NULL)
headers=\
	worker.h \
	$(NULL)

demo: $(sources:.c=.o) Makefile
	gcc -o $@ $(sources:.c=.o) $(shell pkg-config --libs gthread-2.0)

%.o: %.c $(headers) Makefile
	gcc -c -o $@ $< $(shell pkg-config --cflags gthread-2.0)

clean:
	rm -rf *.o
	rm -rf demo
