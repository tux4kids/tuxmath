#include"global.h"
#include"server_handle.h"
int max_slave_server;	//from server_handle.c
static int standby_ports=0; //Keep count on number of ports that are active active 
struct threadID * thread_header;  // from global.h
pthread_mutex_t mutex_variable;  //from global.h


/*================================= from server.c =======================================*/
int setup_server(struct threadID  *instance);
void cleanup_server(struct threadID  *instance);


/*=========================================================================================
	Function responsible for handling threads
  =========================================================================================*/

// fuction responsible for pushing thread in stack and deactivate them 
void push_thread(struct threadID * temp_thread)
{
	pthread_mutex_lock(&mutex_variable);
	if(thread_header==NULL)
	{
		thread_header=temp_thread;
		thread_header->next=NULL;
	}
	else
	{
		temp_thread->next=thread_header;
		thread_header=temp_thread;
	}
	temp_thread->status=0;                                // to make thread stop running till it get call
	++standby_ports;
	pthread_mutex_unlock(&mutex_variable);
}

//function responsible for creation of thread- to be called once 
int  start_new_thread(Uint16 temp_port)  // return 1 on success else 0. 
{
	struct threadID *new_thread=NULL;
	new_thread=malloc(sizeof(struct threadID));// allocating space to new node
	if(new_thread==NULL)
	{
		fprintf(stderr, "Memory Overflow\n");
		return 0;
	}
	else
	{	
		new_thread->info.port=temp_port;  // allocating port number to newly created thread 
		if(sem_init(&(new_thread->binary_sem),0,0)==0)
		new_thread->status=0;   
		pthread_create(&(new_thread->ID),NULL,slave_server,new_thread);
		/*     ---------------- Setup: ---------------------------   */
                if (!setup_server(new_thread))  // From server.c                              
                {
                     fprintf(stderr, "setup_server() failed - exiting.\n");
                     cleanup_server(new_thread); 
                     return 1;
  		} 
		push_thread(new_thread); // pushing newly created thread into stack
	}				
	return 1;
}

// function for activating thread
int activate_thread()
{	
	struct threadID * temp_thread;
	pthread_mutex_lock(&mutex_variable);
	if(thread_header==NULL)
	{
		pthread_mutex_unlock(&mutex_variable);
		return 0;
	}
	else
	{	fprintf(stderr, "Switching thread header\n");
		temp_thread=thread_header;
		thread_header=thread_header->next;
	}
	standby_ports--;
	pthread_mutex_unlock(&mutex_variable);	
	fprintf(stderr, " Unlocking thread\n");
	sem_post(&(temp_thread->binary_sem));
	return 1;
}
void cleanup_thread()
{
	struct threadID *temp_thread;
	pthread_mutex_lock(&mutex_variable);	
	while(thread_header!=NULL)
	{
		temp_thread=thread_header;
		thread_header=thread_header->next;
		cleanup_server(temp_thread);   // From server.c 
		pthread_cancel(temp_thread->ID); //stoping thread
		sem_destroy(&(temp_thread->binary_sem));
		free(temp_thread);
	}
	if(thread_header==NULL)
		fprintf(stderr, "Every thing is clean(Threads)\n");
	
}
int get_active_threads()
{
return (max_slave_server-standby_ports);
}

