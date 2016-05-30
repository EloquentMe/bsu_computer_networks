FILES=main.c
CFLAGS=-g

all: server client

client: $(FILES)
	gcc $(CFLAGS) $^ -o $@

server: client
	cp $^ $@

clean:
	rm -f server client
