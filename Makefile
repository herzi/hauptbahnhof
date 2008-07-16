all: demo

PLATFORM_FLAGS=$(shell uname -s|grep -q Darwin && echo -framework CoreServices)

sources=\
	main.c \
	queue.c \
	worker.c \
	$(NULL)
headers=\
	queue.h \
	worker.h \
	$(NULL)

install: all
	install demo /opt/gtk/bin

demo: $(sources:.c=.o) Makefile
	gcc -ggdb $(PLATFORM_FLAGS) -o $@ $(sources:.c=.o) $(shell pkg-config --libs gthread-2.0)

%.o: %.c $(headers) Makefile
	gcc -ggdb -c -o $@ $< $(shell pkg-config --cflags gthread-2.0)

clean:
	rm -rf *.o
	rm -rf demo
