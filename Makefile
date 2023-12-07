

CFLAGS=-Wall -std=c99

%.o: %.c %.h
	gcc $(CFLAGS) -c $< -o $@

file-properties.o: file-properties.c file-properties.h
	gcc $(CFLAGS) -c $< -o $@

lp25-backup: main.c files-list.o sync.o configuration.o file-properties.o processes.o messages.o utility.o
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f *.o lp25-backup