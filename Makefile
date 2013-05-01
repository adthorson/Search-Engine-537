search-engine: search-engine.o index.o
	gcc -pthread search-engine.o index.o -o search-engine

search-engine.o: search-engine.c
	gcc -pthread -Wall -g -c search-engine.c -o search-engine.o

index.o: index.c
	gcc -pthread -Wall -g -c index.c -o index.o

clean:
	rm -f *.o search-engine
