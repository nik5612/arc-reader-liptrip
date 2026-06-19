CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lpng

OBJS = arc.o dsc.o cbg.o bse.o decrypt.o write.o

all: ethornell

ethornell: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f ethornell *.o
