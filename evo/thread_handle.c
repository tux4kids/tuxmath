#include"global.h"
int active_ports=0; //Keep count on number of ports that are active active 
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
	++active_ports;
	pthread_mutex_unlock(&mutex_variable);
}

//function responsible for creation of thread- to be called once 
int  intialize_thread()  // return 1 on success else 0. 
{
	int i;
	struct threadID *new_thread=NULL;
	pthread_mutex_lock(&mutex_variable);
	for(i=0;i<MAX;i++)
	{
		new_thread=malloc(sizeof(struct threadID));// allocating space to new node
		if(new_thread==NULL)
		{
			printf("\nMemory overflow");
			pthread_mutex_unlock(&mutex_variable);
			return 0;
		}
		else
		{	
			new_thread->client.port=2001+i;  // allocating port number to newly created thread 
			if(sem_init(&(new_thread->binary_sem),0,0)==0)
			new_thread->status=0;   
			pthread_create(&(new_thread->ID),NULL,handle,new_thread);
			// opening port for accepting connections
		if(common_connect_server(&host_ipaddress,&new_thread->client.server_socket,new_thread->client.port,(const char *)NULL)==0)
		{
			printf("\n Not able to open connection");
			return 1;
		}
			pthread_mutex_unlock(&mutex_variable);
			push_thread(new_thread); // pushing newly created thread into stack
		}				
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
	{
		temp_thread=thread_header;
		thread_header=thread_header->next;
	}
	active_ports--;
	pthread_mutex_unlock(&mutex_variable);	
	printf("\n Unlocking");
	sem_post(&(temp_thread->binary_sem));
	sem_post(&(temp_thread->binary_sem));
}
void cleanup_thread()
{
	struct threadID *temp_thread;
	pthread_mutex_lock(&mutex_variable);	
	while(thread_header!=NULL)
	{
		temp_thread=thread_header;
		thread_header=thread_header->next;
		SDLNet_TCP_Close(temp_thread->client.server_socket);
		pthread_cancel(temp_thread->ID); //stoping thread
		sem_destroy(&(temp_thread->binary_sem));
		free(temp_thread);
	}
	if(thread_header==NULL)
		printf("\n Great every thing is cleaned");
	
}
