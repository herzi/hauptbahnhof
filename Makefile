all: demo

demo: main.c Makefile
	gcc -o $@ $<

clean:
	rm -rf demo
