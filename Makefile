SOURCES = example.c libmpdclient.c

connect: $(SOURCES)
	cc -Wall -ggdb -o example $(SOURCES)

clean:
	rm -f *.o example
