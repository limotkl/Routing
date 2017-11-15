all: client haha

haha:
	gcc routing.c -o 5.out -lpthread
client:
	gcc client.c -o client.out -lpthread

clean:
	rm 5.out
	rm client.out

