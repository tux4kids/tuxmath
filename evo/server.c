#include"server_handle.h"
int main()
{
	void * exit_status;
	pthread_t *thread;
	
	thread=SDLserver_init(2,2000);
	pthread_join(*thread,&exit_status);
	SDLserver_quit();
}

