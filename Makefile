all: demo

demo: main.c Makefile
	gcc -o $@ $< $(shell pkg-config --cflags --libs gthread-2.0)

clean:
	rm -rf demo
