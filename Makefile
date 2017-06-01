PROG=audio.o
CC=gcc
CFLAGS= -lasound
all: $(PROG)


$(PROG): main.c audio.c
	gcc main.c audio.c $(CFLAGS) -o $@
clean:
	rm -rf $(PROG)

