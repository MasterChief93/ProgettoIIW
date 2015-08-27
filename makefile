CC=gcc
CFLAGS = -lm -pthread -lsqlite3 -I. -l wurfl `pkg-config --cflags --libs MagickWand` -DSQLITE_THREADSAFE=1
DEPS = processwork.h threadwork.h fileman.h db.h garbage.h parsing.h resizing.h
OBJ = main.o processwork.o threadwork.o config.o db.o garbage.o parsing.o resizing.c

%.o: %.c $(DEPS)
	$(CC) -Wall -O2 -Wextra -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -Wall -O2 -Wextra -o $@ $^ $(CFLAGS)