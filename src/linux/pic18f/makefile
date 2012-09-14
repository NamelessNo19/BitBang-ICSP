CC=gcc
CFLAGS=-I.
DEPS = serialcon.h pic18fcon.h hexdec.h
OBJ = picicsp.o serialcon.o pic18fcon.o hexdec.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

picicsp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm ./*.o
