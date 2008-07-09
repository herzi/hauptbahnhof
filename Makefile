all: demo

sources=\
	main.c \
	$(NULL)
headers=\
	$(NULL)

demo: $(sources:.c=.o) Makefile
	gcc -o $@ $(sources:.c=.o) $(shell pkg-config --libs gthread-2.0)

.c.o: $(headers) Makefile
	gcc -c -o $@ $< $(shell pkg-config --cflags gthread-2.0)

clean:
	rm -rf demo
