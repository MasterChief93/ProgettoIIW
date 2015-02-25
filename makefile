CC=gcc
CFLAGS = -lm -pthread -I.
DEPS = processwork.h threadwork.h
OBJ = main.o processwork.o threadwork.o

%.o: %.c $(DEPS)
	$(CC) -Wall -O2 -Wextra -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -Wall -O2 -Wextra -o $@ $^ $(CFLAGS)