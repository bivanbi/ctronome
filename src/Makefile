GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"
CC = gcc
OBJS = ctronome.o dsp.o routines.o
CFLAGS = -DVERSION=\"$(GIT_VERSION)\"

all: ctronome

%.o: %.c ctronome.h routines.h dsp.h
	$(CC) $(CFLAGS) -c $<

ctronome: $(OBJS)
	$(CC) -o ctronome $(OBJS)

clean:
	rm -f *.o ctronome
