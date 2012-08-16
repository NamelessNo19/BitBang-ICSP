CC=gcc
CFLAGS=-I.
DEPS = serialcon.h attiny13con.h hexdec.h
OBJ = icsp.o serialcon.o attiny13con.o hexdec.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

icsp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm ./*.o
