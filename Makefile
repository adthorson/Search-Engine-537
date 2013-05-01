search-engine: search-engine.o index.o
        gcc search-engine.o index.o -o search-engine

search-engine.o: search-engine.c
        gcc -Wall -g -c search-engine.c -o search-engine.o

index.o: index.c
        gcc -Wall -g -c index.c -o index.o

clean:
      	rm -f *.o search-engine
