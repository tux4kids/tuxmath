
server: server.c thread_handle.c server_handle.c client
	gcc -c thread_handle.c server_handle.c
	ar rcs sdl_server.a thread_handle.o server_handle.o 
	gcc server.c sdl_server.a -o server -lSDL_net
	
client:client.c
	gcc client.c -o client -lSDL_net 

.PHONY: clean
clean:
	rm server
	rm client
	rm *.o
	rm *.a
