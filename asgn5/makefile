library: mysocket.o queue.o
	ar rcs libmysocket.a mysocket.o queue.o

run: client.c server.c  
	gcc -L. server.c -lmysocket -o server
	gcc -L. client.c -lmysocket -o client

mysocket.o: mysocket.h mysocket.c queue.o
	gcc -c mysocket.c queue.o -o mysocket.o

queue.o: queue.h queue.c
	gcc -c queue.c -o queue.o

clean:
	rm *.o client server