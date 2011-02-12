
server: server.c thread_handle.c client.c
	gcc -c thread_handle.c
	gcc server.c thread_handle.o -o server -lSDL_net
	gcc client.c -o client -lSDL_net 

.PHONY: clean
clean:
	rm server
	rm client
	rm *.o
