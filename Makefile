all: demo

sources=\
	main.c \
	$(NULL)

demo: $(sources) Makefile
	gcc -o $@ $(sources) $(shell pkg-config --cflags --libs gthread-2.0)

clean:
	rm -rf demo
