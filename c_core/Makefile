CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lm

all: search_engine

search_engine: main.o trie.o inverted_index.o search.o tfidf.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c trie.h inverted_index.h search.h utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

trie.o: trie.c trie.h
	$(CC) $(CFLAGS) -c -o $@ $<

inverted_index.o: inverted_index.c inverted_index.h
	$(CC) $(CFLAGS) -c -o $@ $<

search.o: search.c search.h trie.h inverted_index.h tfidf.h
	$(CC) $(CFLAGS) -c -o $@ $<

tfidf.o: tfidf.c tfidf.h inverted_index.h
	$(CC) $(CFLAGS) -c -o $@ $<

utils.o: utils.c utils.h trie.h inverted_index.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	del /f /q *.o search_engine.exe