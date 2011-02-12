#ifndef _THREAD_HANDLE_H
#define _THREAD_HANDLE_H
// fuction responsible for pushing thread in stack and deactivate them 
void push_thread(struct threadID * temp_thread);

//function responsible for creation of thread- to be called once 
int  intialize_thread();  // return 1 on success else 0. 

// function for activating thread
int activate_thread();

void cleanup_thread();
#endif

