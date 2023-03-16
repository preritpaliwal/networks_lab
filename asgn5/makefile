

run: client.c server.c mysocket.o queue.o
	gcc client.c mysocket.o queue.o -o client
	gcc server.c mysocket.o queue.o -o server

library: mysocket.o
	ar rcs libmsocket.a mysocket.o queue.o

mysocket.o: mysocket.h mysocket.c queue.o
	gcc mysocket.c queue.o -c mysocket.o

queue.o: queue.h queue.c
	gcc queue.c -c queue.o

clean:
	rm *.o client server