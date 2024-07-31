GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"
CC = gcc
OBJS = ctronome.o dsp.o routines.o defaults.o
CFLAGS = -Wall -DVERSION=\"$(GIT_VERSION)\"

all: ctronome

%.o: %.c ctronome.h routines.h dsp.h defaults.h
	$(CC) $(CFLAGS) -c $<

ctronome: $(OBJS)
	$(CC) -o ctronome $(OBJS)

clean:
	rm -f *.o ctronome
