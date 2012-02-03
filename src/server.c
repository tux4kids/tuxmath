/*
   server.c:

   Server program for LAN-based play in Tux,of Math Command.

NOTE: This file was initially based on example code from The Game
Programming Wiki (http://gpwiki.org), in a tutorial covered by the
GNU Free Documentation License 1.2. No invariant sections were 
indicated, and no separate license for the example code was listed.
The author was also not listed. AFAICT,this scenario allows incorporation
of derivative works into a GPLv3+ project like TuxMath.  FWIW, virtually
none of the tutorial code is still present here - David Bruce 

Copyright 2009, 2010, 2011.
Authors: Akash Gangil, David Bruce
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

server.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

Tuxmath is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Tuxmath is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/




/* This must come before #ifdef HAVE_LIBSDL_NET to get "config.h" */
#include "globals.h"

#ifdef HAVE_LIBSDL_NET

#include "options.h" 
#include "server.h" 
#include "transtruct.h"
#include "mathcards.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h> 
#include <sys/types.h>  
#include <unistd.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#define MAX_ARGS 16
#define SRV_QUEST_INTERVAL 2000

typedef struct srv_game_type {
    char lesson_name[NAME_SIZE];
    int wave;
    int active_quests;        //Number of questions currently "in play"
    int max_quests_on_screen;
    int quests_in_wave;
    int rem_in_wave;          //Number still to be issued in wave
}srv_game_type;




/*  -----------  Local function prototypes:   ------------  */

// setup and cleanup:
int setup_server(int thread_id_no);
void cleanup_server(int thread_id_no);
void server_handle_command_args(int argc, char* argv[]);
void* run_server_local_args(void* data);

// top level functions in main loop:
void check_UDP(int thread_id_no);
void update_clients(int thread_id_no);
int server_check_messages(int thread_id_no);
void server_update_game(int thread_id_no);
void server_check_stdin(int thread_id_no);
// client management utilities:
int find_vacant_client(int thread_id_no);
void remove_client(int thread_id_no, int i);
void check_game_clients(int thread_id_no);

// message reception:
int handle_client_game_msg(int thread_id_no, int i, char* buffer);
void handle_client_nongame_msg(int thread_id_no, int i, char* buffer);
int msg_set_name(int thread_id_no, int i, char* buf);
void msg_socket_index(int thread_id_no, int i, char* buf);
void start_game(int thread_id_no);
void end_game(int thread_id_no);
void game_msg_correct_answer(int thread_id_no, int i, char* inbuf);
void game_msg_wrong_answer(int thread_id_no, int i, char* inbuf);
void game_msg_quit(int thread_id_no, int i);
void game_msg_exit(int thread_id_no, int i);
int calc_score(int difficulty, float t);

//message sending:
int add_question(int thread_id_no, MC_FlashCard* fc);
int remove_question(int thread_id_no, int quest_id, int answered_by);
int send_counter_updates(int thread_id_no);
int send_player_updates(int thread_id_no);
//int SendQuestion(MC_FlashCard flash, TCPsocket client_sock);
int SendMessage(int message, int ques_id, char* name, TCPsocket client_sock);
int player_msg(int thread_id_no, int i, char* msg);
void broadcast_msg(int thread_id_no, char* msg);
int transmit(int thread_id_no, int i, char* msg);
int transmit_all(int thread_id_no, char* msg);

// For non-blocking input:
int read_stdin_nonblock(char* buf, size_t max_length);


// not really deprecated but not done in response to 
// client message --needs better name:
void game_msg_next_question(int thread_id_no);

/* global mathgame struct for lan game: */
extern MC_MathGame* lan_game_settings;  //TODO Deepak:- see its effect and change it accordingly

/*  ------------   "Local globals" for server.c: ----------  */
char server_name[NAME_SIZE];  /* User-visible name for server selection  */
int need_server_name = 1;     /* Always request server name */
static int game_in_progress = 0;
static int server_running = 0;
static int quit = 0;
static int ignore_stdin = 0;    //TODO not needed as all work is done in threads

/* used for keeping record of every instance of a thread running within a server */
struct threadID   
{
    UDPsocket udpsock ;              /* Used to listen for client's server autodetection           */
    TCPsocket server_sock;    /* Socket descriptor for server to accept client TCP sockets. */
    IPaddress ip;
    SDLNet_SocketSet client_set;
    struct client_type client[MAX_CLIENTS];  //TODO Deepak removed static from it as they can't be declared inside it. might result problem in future 
    int num_clients;
    struct srv_game_type srv_game;
};
struct threadID slave_thread[2]; //TODO it might have to be replaced with a pointer pointing to head of the stack when integrating thread in it.

// These are to allow the server to be invoked in a thread
// with the same syntax as used to launch it as a standalone
// program:
int local_argc;                                         //TODO Deepak ( I have to dig up more to see there effect )
char* local_argv[MAX_ARGS];
char local_argv_storage[MAX_ARGS][256];






/* The previous contents of main() are wrapped into this function to   */
/* allow the server to be run as a function in a process or thread     */
/* within another program.  main() is now in a separate file,          */
/* servermain.c, that consists solely of a call to RunServer().        */

int RunServer(int argc, char* argv[])
{ 
    Uint32 timer = 0;
    ignore_stdin = 0;
    int frame = 0;

    fprintf(stderr, "Started tuxmathserver, waiting for client to connect:\n>\n");

    server_handle_command_args(argc, argv);

    /*     ---------------- Setup: ---------------------------   */
    if (!setup_server(0)) //FIXME Deepak its hard coded.
    {
        fprintf(stderr, "setup_server() failed - exiting.\n");
        cleanup_server(0);  //FIXME Deepak its hard coded.
        return EXIT_FAILURE;
    }

    DEBUGMSG(debug_lan, "In RunServer(), server_name is: %s\n", server_name);

    server_running = 1;
    quit = 0;

    fprintf(stderr, "Waiting for clients to connect:\n>");
    fflush(stdout);


    /*    ------------- Main server loop:  ------------------   */
    while (!quit)
    {
        DEBUGCODE(debug_lan)
        {
            if(frame % 1000 == 0)
                fprintf(stderr, "server running\n");
        }

        /* Respond to any clients pinging us to find the server: */
        check_UDP(0);    //FIXME Deepak its hard coded.
        /* Now we check to see if anyone is trying to connect. */
        update_clients(0); //FIXME Deepak its hard coded.
        /* Check for any pending messages from clients already connected: */
        server_check_messages(0);  //FIXME Deepak its hard coded.
        /* Handle any game updates not driven by received messages:  */
        server_update_game(0);     //FIXME Deepak its hard coded
        /* Check for command line input, if appropriate: */
        server_check_stdin(0);   // FIXME Deepak its hard coded
        /* Limit frame rate to keep from eating all CPU: */
        /* NOTE almost certainly could make this longer wtihout noticably */
        /* affecting performance, but even throttling to 1 msec/loop cuts */
        /* CPU from 100% to ~2% on my desktop - DSB                       */
        T4K_Throttle(5, &timer);  //min loop time 5 msec
        frame++;
    }

    server_running = 0;

    /*   -----  Free resources before exiting: -------    */
    cleanup_server(0); //FIXME Deepak its hard coded.

    return EXIT_SUCCESS;
}



/* If we can't use pthreads, we use this function   */
/* to launch the server as a separate program using */
/* the C library system() call                      */
int RunServer_prog(int argc, char* argv[])
{
    char buf[256];
    int i;

    /* Construct command-line argument string from argc and argv:   */
    /* NOTE this is not safe from buffer overflow - do              */
    /* not use with user-supplied arguments.                        */
    snprintf(buf, 256, "tuxmathserver ");
    for(i = 1; i < argc; i++)
    {
        strncat(buf, argv[i], 256);
        strncat(buf, " ", 256);
    }
    /* Add '&' to make it non-blocking: */
    strncat(buf, "&", 256);

    DEBUGMSG(debug_lan, "RunServer_prog() - launching standalone " 
            "tuxmathserver program with \"system(%s)\"\n", buf);

    return system(buf);
}

/*
 * RunServer_pthread() is the preferred way to run the tuxmath server,
 */

#ifdef HAVE_PTHREAD_H
int RunServer_pthread(int argc, char* argv[])
{
    pthread_t server_thread;
    int i;

    DEBUGMSG(debug_lan, "In RunServer_pthread():\n"
            "argc = %d\n"
            "argv[0] = %s\n"
            "argv[1] = %s\n"
            "argv[2] = %s\n",
            argc, argv[0], argv[1], argv[2]);

    /* We can only pass a single arg into the new thread, but it shares  */
    /* the same address space, so we save argc and argv locally instead: */
    local_argc = argc;
    for(i = 0; i < argc && i < MAX_ARGS; i++)
    {
        strncpy(local_argv_storage[i], argv[i], 256);     
        local_argv[i] = local_argv_storage[i];
    }
    /* We need to tell the server not to poll stdin: */
    if(argc < MAX_ARGS)
    {
        strncpy(local_argv_storage[argc], "--ignore-stdin", 256);         
        local_argv[argc] = local_argv_storage[argc];
        local_argc++;
    }
    else fprintf(stderr, "In RunServer_pthread() - warning - could not append '--ignore-stdin'\n");

    DEBUGMSG(debug_lan, "In RunServer_pthread():\n"
            "local_argc = %d\n"
            "local_argv[0] = %s\n"
            "local_argv[1] = %s\n"
            "local_argv[2] = %s\n"
            "local_argv[3] = %s\n",
            local_argc, local_argv[0], local_argv[1], local_argv[2], local_argv[3]);


    if(pthread_create(&server_thread, NULL, run_server_local_args, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return -1;
    }
    return 0;
}

void* run_server_local_args(void* data)
{
    DEBUGMSG(debug_lan, "In RunServer_pthread():\n"
            "local_argc = %d\n"
            "local_argv[0] = %s\n"
            "local_argv[1] = %s\n"
            "local_argv[2] = %s\n"
            "local_argv[3] = %s\n",
            local_argc, local_argv[0], local_argv[1], local_argv[2], local_argv[3]);

    RunServer(local_argc, local_argv);
    pthread_exit(NULL);
    return NULL;
}

#endif


/* Find out if server is already running within this program: */
int OurServerRunning(void)
{
    return server_running;
}

/* Find out if desired port is available: */
/* NOTE - the purpose of this function is to check first if the server
 * is likely to start successfully, so we don't confusingly accept the
 * server name and desired lesson, only to have the server startup fail - DSB */
int PortAvailable(Uint16 port)
{
    IPaddress tmp_ip;
    TCPsocket tmp_sock = NULL;
    int available = 0;

    if (SDLNet_ResolveHost(&tmp_ip, NULL, port) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }

    /* Try to open a socket on our machine with desired port,
     * record whether we succeed, and close the socket again
     * so we can connect for real.
     */
    tmp_sock = SDLNet_TCP_Open(&tmp_ip);

    if (!tmp_sock)
        available = 0;
    else
    {
        available = 1;
        SDLNet_TCP_Close(tmp_sock);
    }

    return available;
}


/* Find out if game is already in progress: */
int SrvrGameInProgress(void)
{
    return game_in_progress;
}

/* FIXME make these more civilized - notify players, clean up game
 * properly, and so forth.
 */

/* Stop Server */
void StopServer(void)
{
    StopSrvrGame(0);   // FIXME Deepak its hard coded. StopServer is called from somewhere else
    quit = 1;
}


/* Stop currently running game: */
void StopSrvrGame(int thread_id_no)
{
    end_game(thread_id_no);
    //TODO send notifications to players
}




/*********************************************************************/
/*  "Private" (to server.c) functions                                */
/*********************************************************************/


/*  ----- Setup and Cleanup:  ------------------- */

/* NOTE: these functions no longer include initialization or quitting of 
 * SDL or SDL_net.  These things needed to be handled within tuxmath
 * itself (for the pthread-based server) or within servermain.c, for the
 * standalone program.  We want to be able to shut down the pthread-based
 * server without crashing the rest of tuxmath - DSB
 */

// setup_server() - all the things needed to get server running:
int setup_server(int thread_id_no)
{
    Uint32 timer = 0;

    slave_thread[thread_id_no].num_clients=0; // To ensure no garbage value is used. 

    /* Resolving the host using NULL make network interface to listen */
    if (SDLNet_ResolveHost(&(slave_thread[thread_id_no].ip), NULL, DEFAULT_PORT) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }

    /* Open a connection with the IP provided (listen on the host's port) */
    if (!(slave_thread[thread_id_no].server_sock = SDLNet_TCP_Open(&(slave_thread[thread_id_no].ip)) ) )
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    slave_thread[thread_id_no].client_set = SDLNet_AllocSocketSet(MAX_CLIENTS);
    if(!(slave_thread[thread_id_no].client_set) )
    { 
        fprintf(stderr, "SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        return 0;
    }

    //this sets up our mathcards "library" with hard-coded defaults - no
    //settings read from config file here as of yet:
    if (!MC_Initialize(lan_game_settings))
    {
        fprintf(stderr, "Could not initialize MathCards\n");
        return 0;
    }

    /* Get server name: */
    /* We use default name after 30 sec timeout if no name entered. */
    /* FIXME we should save this to disc so it doesn't */
    /* have to be entered every time.                  */
    if(need_server_name)
    {
        Uint32 timeout = SDL_GetTicks() + SERVER_NAME_TIMEOUT;
        int name_recvd = 0;
        server_name[0] = '\0';

        /* We can use fcntl() on Linux/Unix plaforms: */
#ifdef HAVE_FCNTL   
        fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

        fprintf(stderr, "Enter the SERVER's NAME: \n>");
        fflush(stdout);

        while(!name_recvd && (SDL_GetTicks() < timeout))
        {
            if(read_stdin_nonblock(server_name, NAME_SIZE))
                name_recvd = 1;
            T4K_Throttle(10, &timer);
        }
        if(!name_recvd)
            fprintf(stderr, "No name entered within timeout, will use default: %s\n",
                    DEFAULT_SERVER_NAME);

        /* If no nickname received, use default: */
        if(strlen(server_name) == 0)
            strncpy(server_name, DEFAULT_SERVER_NAME, NAME_SIZE);
#else
        /* HACK - until we figure out how to do nonblocking stdin
         * in Windows, we just stick in the default name:
         */
        DEBUGMSG(debug_lan, "fnctl() not available, initially using default server name\n");
        DEBUGMSG(debug_lan, "Default name is: DEFAULT_SERVER_NAME\n");
        strncpy(server_name, DEFAULT_SERVER_NAME, NAME_SIZE);
#endif

        DEBUGMSG(debug_lan, "server_name has been set to: %s\n", server_name);
    }


    // Zero out our client list:
    {
        int i = 0;
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            slave_thread[thread_id_no].client[i].game_ready = 0;   /* waiting for user to OK game start */
            strncpy(slave_thread[thread_id_no].client[i].name, _("Await player name"), NAME_SIZE);   /* no nicknames yet                  */
            slave_thread[thread_id_no].client[i].sock = NULL;      /* sockets start out unconnected     */
            slave_thread[thread_id_no].client[i].score = 0;
        }
    }


    //Now open a UDP socket to listen for clients broadcasting to find the server:
    slave_thread[thread_id_no].udpsock = SDLNet_UDP_Open(DEFAULT_PORT);
    if(!slave_thread[thread_id_no].udpsock)
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    // Indicates success:
    return 1;
}



//Free resources, closing sockets, and so forth:
void cleanup_server(int thread_id_no)
{
    int i;
    /* Close the client socket(s) */

    for(i = 0; i < MAX_CLIENTS; i++)
    {
        if(slave_thread[thread_id_no].client[i].sock != NULL)
        {
            SDLNet_TCP_Close(slave_thread[thread_id_no].client[i].sock);    //close all the client sockets one by one
            slave_thread[thread_id_no].client[i].sock = NULL;               // So we don't segfault in case cleanup()
        }                                      // somehow gets called more than once.
    } 

    if (slave_thread[thread_id_no].client_set != NULL)
    {
        SDLNet_FreeSocketSet(slave_thread[thread_id_no].client_set);    //releasing the memory of the client socket set
        slave_thread[thread_id_no].client_set = NULL;                   //this helps us remember that this set is not allocated
    } 

    if(slave_thread[thread_id_no].server_sock != NULL)
    {
        SDLNet_TCP_Close(slave_thread[thread_id_no].server_sock);
        slave_thread[thread_id_no].server_sock = NULL;
    }

    if(slave_thread[thread_id_no].udpsock != NULL)
    {
        SDLNet_UDP_Close(slave_thread[thread_id_no].udpsock);
        slave_thread[thread_id_no].udpsock = NULL;
    }
}


/* Handle any arguments passed from command line */
void server_handle_command_args(int argc, char* argv[])
{
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            /* Display help message: */
            fprintf(stderr, "\n");
            cleanup_server(0);                                //FIXME Deepak its hard coded.
            exit(0);
        }
        else if (strcmp(argv[i], "--debug-lan") == 0)
        {
            debug_status |= debug_lan;
        }
        else if (strcmp(argv[i], "--ignore-stdin") == 0)
        {
            ignore_stdin = 1;
        }

        else if (strcmp(argv[i], "--copyright") == 0 ||
                strcmp(argv[i], "-c") == 0)
        {
            printf(
                    "\n\"Tux, of Math Command Server\" version " VERSION ", Copyright (C) 2009, 2010\n"
                    "David Bruce, Akash Gangil, and the Tux4Kids Project.\n"
                    "This program is free software; you can redistribute it and/or\n"
                    "modify it under the terms of the GNU General Public License\n"
                    "as published by the Free Software Foundation.  See COPYING.txt\n"
                    "\n"
                    "This program is distributed in the hope that it will be useful,\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                    "\n");

            cleanup_server(0);             //FIXME Deepak its hard coded.
            exit(0);
        }
        else if (strcmp(argv[i], "--usage") == 0 ||
                strcmp(argv[i], "-u") == 0)
        {
            /* Display (happy) usage: */
            // TODO write usage() for server
            //      usage(0, argv[0]);
        }
        else if ((strcmp(argv[i], "--name") == 0 || strcmp(argv[i], "-n") == 0)
                && (i + 1 < argc))
        {
            strncpy(server_name, argv[i + 1], NAME_SIZE);
            need_server_name = 0;
        }
    }
}


// ----------- Top level functions in main loop ---------------:

//check_UDP() is the server side of the client-server autodetection system.
//When a client wants to connect, it sends a UDP broadcast to the local
//network on this port, and the server sends a response.
//The client will then try to open a TCP socket at the server's ip address,
//which will be picked up in update_clients() below.
void check_UDP(int thread_id_no)
{
    int recvd = 0;
    UDPpacket* in = NULL;

    if(slave_thread[thread_id_no].udpsock == NULL)
    {
        fprintf(stderr, "warning - check_UDP() called but udpsock == NULL\n");
        return;
    }

    in = SDLNet_AllocPacket(NET_BUF_LEN);
    recvd = SDLNet_UDP_Recv(slave_thread[thread_id_no].udpsock, in);

    if(recvd > 0)
    {   
        DEBUGMSG(debug_lan, "check_UDP() received packet: %s\n", (char*)in->data);  
        // See if packet contains identifying string:
        if(strncmp((char*)in->data, "TUXMATH_CLIENT", strlen("TUXMATH_CLIENT")) == 0)
        {
            UDPpacket* out;
            int sent = 0;
            char buf[NET_BUF_LEN];
            // Send "I am here" reply so client knows where to connect socket,
            // with configurable identifying string so user can distinguish 
            // between multiple servers on same network (e.g. "Mrs. Adams' Class");
            out = SDLNet_AllocPacket(NET_BUF_LEN); 
            snprintf(buf, NET_BUF_LEN, "%s\t%s\t%s",
                    "TUXMATH_SERVER", server_name, Opts_LessonTitle());
            snprintf(out->data, NET_BUF_LEN, "%s", buf);
            out->len = strlen(buf) + 1;
            out->address.host = in->address.host;
            out->address.port = in->address.port;
            sent = SDLNet_UDP_Send(slave_thread[thread_id_no].udpsock, -1, out);
            SDLNet_FreePacket(out);
        }
    }

    SDLNet_FreePacket(in);
}




//update_clients() sees if anyone is trying to connect, and connects if a slot
//is open and the game is not in progress. The purpose is to make sure our
//client set accurately reflects the current state.
void update_clients(int thread_id_no)
{
    TCPsocket temp_sock = NULL;        /* Just used when client can't be accepted */
    int slot = 0;
    int sockets_used = 0;
    char buffer[NET_BUF_LEN];

    /* See if we have a pending connection: */
    temp_sock = SDLNet_TCP_Accept(slave_thread[thread_id_no].server_sock);
    if (!temp_sock)  /* No one waiting to join - do nothing */
    {
        return;   // Leave num_clients unchanged
    }

    // See if any slots are available:
    slot = find_vacant_client(thread_id_no);
    if (slot == -1) /* No vacancies: */
    {
        snprintf(buffer, NET_BUF_LEN, 
                "%s\t%s",
                "PLAYER_MSG",
                "Sorry, already have maximum number of clients connected");
        SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
        //hang up:
        SDLNet_TCP_Close(temp_sock);
        temp_sock = NULL;

        DEBUGMSG(debug_lan, "update_clients() - no vacant slot found\n");

        return;   // Leave num_clients unchanged
    }

    //If everyone is disconnected, game no longer in progress:
    check_game_clients(thread_id_no); 

    // If game already started, send our regrets:
    if(game_in_progress)
    {
        snprintf(buffer, NET_BUF_LEN, 
                "%s",
                "GAME_IN_PROGRESS");
        SDLNet_TCP_Send(temp_sock, buffer, NET_BUF_LEN);
        //hang up:
        SDLNet_TCP_Close(temp_sock);
        temp_sock = NULL;

        DEBUGMSG(debug_lan, "update_clients() - game already started\n");

        return;   // Leave num_clients unchanged
    }

    // If we get to here, we have room for the new connection and the
    // game is not in progress, so we connect:
    DEBUGMSG(debug_lan, "creating connection for client[%d].sock:\n", slot);

    slave_thread[thread_id_no].client[slot].sock = temp_sock;

    /* Add client socket to set: */
    sockets_used = SDLNet_TCP_AddSocket(slave_thread[thread_id_no].client_set, slave_thread[thread_id_no].client[slot].sock);
    if(sockets_used == -1) //No way this should happen
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        cleanup_server(0);                            //FIXME Deepak its hard coded.
        exit(EXIT_FAILURE);
    }

    /* At this point num_clients can be updated: */
    slave_thread[thread_id_no].num_clients = sockets_used;

    /* Now we can communicate with the client using slave_thread[thread_id_no].client[i].sock socket */
    /* serv_sock will remain opened waiting other connections.            */

    /* Send message informing client of successful connection:            */
    msg_socket_index(thread_id_no, slot, buffer);
    /* Now tell rest of clients that another has joined: */
    send_player_updates(thread_id_no);
    /* Get the remote address */
    DEBUGCODE(debug_lan)
    {
        IPaddress* client_ip = NULL;
        client_ip = SDLNet_TCP_GetPeerAddress(slave_thread[thread_id_no].client[slot].sock);

        fprintf(stderr, "num_clients = %d\n", slave_thread[thread_id_no].num_clients);
        if (client_ip != NULL)
            /* Print the address, converting in the host format */
        {
            fprintf(stderr, "Client connected\n>\n");
            fprintf(stderr, "Client: IP = %x, Port = %d\n",
                    SDLNet_Read32(&client_ip->host),
                    SDLNet_Read16(&client_ip->port));
        }
        else
            fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
    }

    return;
}



// check_messages() is where we look at the client socket set to see which 
// have sent us messages. This function is used in each server loop whether
// or not a math game is in progress (although we expect different messages
// during a game from those encountered outside of a game)

int server_check_messages(int thread_id_no)
{
    int actives = 0, i = 0;
    int ready_found = 0;
    char buffer[NET_BUF_LEN];

    /* Check the client socket set for activity: */
    actives = SDLNet_CheckSockets(slave_thread[thread_id_no].client_set, 0);
    //  fprintf(stderr, "in check_messages(), actives = %d\n", actives);
    if(actives == -1)
    {
        fprintf(stderr, "In server_check_messages(), SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        //most of the time this is a system error, where perror might help you.
        perror("In server_check_messages(), SDLNet_CheckSockets");
    }

    else if(actives) 
    {
        DEBUGMSG(debug_lan, "There are %d sockets with activity\n", actives);

        // check all sockets with SDLNet_SocketReady and handle the active ones.
        // NOTE we have to check all the slots in the set because
        // the set will become discontinuous if someone disconnects
        // NOTE this will only pick up the first message for each socket each time
        // check_messages() called - probably OK if we just get it next time through.
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            if((slave_thread[thread_id_no].client[i].sock != NULL)
                    && (SDLNet_SocketReady(slave_thread[thread_id_no].client[i].sock))) 
            { 
                ready_found++;

                DEBUGMSG(debug_lan, "client socket %d is ready\n", i);

                if (SDLNet_TCP_Recv(slave_thread[thread_id_no].client[i].sock, buffer, NET_BUF_LEN) > 0)
                {
                    DEBUGMSG(debug_lan, "buffer received from client %d is: %s\n", i, buffer);

                    /* Here we pass the client number and the message buffer */
                    /* to a suitable function for further action:                */
                    if(game_in_progress)
                    {
                        handle_client_game_msg(thread_id_no, i, buffer);
                    }
                    else
                    {
                        handle_client_nongame_msg(thread_id_no, i, buffer);
                    }
                    // See if game is ended because everyone has left:
                    check_game_clients(thread_id_no); 
                }
                else  // Socket activity but cannot receive - client invalid
                {
                    fprintf(stderr, "Client %d active but receive failed - apparently disconnected\n>\n", i);
                    remove_client(thread_id_no,i);
                }
            }
        }  // end of for() loop - all client sockets checked
        check_game_clients(thread_id_no); //APPARENTLY checking one more time "just in case"???
        // Make sure all the active sockets reported by SDLNet_CheckSockets()
        // are accounted for:

        if(actives > ready_found)
        {
            fprintf(stderr, "Warning: SDLNet_CheckSockets() reported %d active sockets,\n"
                    "but only %d detected by SDLNet_SocketReady()\n", actives, ready_found);
            //Presently, this just runs ping_client() on all the sockets:
            //test_connections();
        }
    } 
    return 1;
}


void server_check_stdin(int thread_id_no)
{
    char buffer[NET_BUF_LEN];
    /* Get out if we are ignoring stdin, e.g. thread in tuxmath gui program: */
    if(ignore_stdin)
        return;
    /* Otherwise handle any new messages from command line: */
    if(read_stdin_nonblock(buffer, NET_BUF_LEN))
    { 
        if( (strncmp(buffer, "exit", 4) == 0) // shut down server thread or prog
                ||(strncmp(buffer, "quit", 4) == 0))

        {
            //FIXME notify clients that we are shutting down
            quit = 1;
        }
        else if (strncmp(buffer, "endgame", 7) == 0) // stop game leaving server running
        {
            end_game(thread_id_no);
        }
        else
        {
            fprintf(stderr, "Command not recognized.\n");
        }
    }
}


// client management utilities:

//Returns the index of the first vacant client, or -1 if all clients full
int find_vacant_client(int thread_id_no)
{
    int i = 0;
    while (slave_thread[thread_id_no].client[i].sock && i < MAX_CLIENTS)
        i++;
    if (i == MAX_CLIENTS)
    {
        fprintf(stderr, "All clients checked, none vacant\n");
        i = -1;
    }
    return i;
}


void remove_client(int thread_id_no, int i)
{
    int j;
    char buf[256];

    fprintf(stderr, "Removing client[%d] - name: %s\n>\n", i, slave_thread[thread_id_no].client[i].name);
    sprintf(buf, "PLAYER_LEFT\t%d", i);

    for(j=0; j<MAX_CLIENTS; j++) {
        if(j != i && slave_thread[thread_id_no].client[j].sock) {
            SDLNet_TCP_Send(slave_thread[thread_id_no].client[j].sock, buf, strlen(buf) + 1);
        }
    }

    SDLNet_TCP_DelSocket(slave_thread[thread_id_no].client_set, slave_thread[thread_id_no].client[i].sock);

    if(slave_thread[thread_id_no].client[i].sock != NULL)
        SDLNet_TCP_Close(slave_thread[thread_id_no].client[i].sock);

    slave_thread[thread_id_no].client[i].sock = NULL;  
    slave_thread[thread_id_no].client[i].game_ready = 0;
    slave_thread[thread_id_no].client[i].name[0] = '\0';
}


// check_game_clients() reviews the game_ready flags of all the connected
// clients to determine if a new game is started, or if an old game needs
// to be ended because all the players have left.  If it finds both "playing"
// and "nonplaying clients", it leaves game_in_progress unchanged.

// TODO this is not very sophisticated, and only supports one game at a time.
// We may want to make this extensible to multiple simultaneous games, perhaps
// with each game in its own thread with its own socket set and mathcards instance.
// FIXME we need to do more than just toggle game_in_progress - should have
// start_game() and end_game() functions that make sure mathcards is 
// properly set up or cleaned up.
void check_game_clients(int thread_id_no)
{
    int i = 0;

    //If the game is already started, we leave it running as long as at least
    //one client is both connected and willing to play:
    if(game_in_progress)
    {
        int someone_still_playing = 0;
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            if((slave_thread[thread_id_no].client[i].sock != NULL)
                    && slave_thread[thread_id_no].client[i].game_ready)
            {
                someone_still_playing = 1;
                break;
            }
        }

        if(!someone_still_playing)
        {
            DEBUGMSG(debug_lan, "All the clients have left the game, setting game_in_progress = 0.\n");

            /* Now make sure all clients are closed: */ 
            for(i = 0; i < MAX_CLIENTS; i++)
            {
                SDLNet_TCP_Close(slave_thread[thread_id_no].client[i].sock);
                slave_thread[thread_id_no].client[i].sock = NULL;
                slave_thread[thread_id_no].client[i].game_ready = 0;
            }

            game_in_progress = 0;
            end_game(thread_id_no);
        }
    }
    //If the game hasn't started yet, we only start it 
    //if all connected clients are ready:
    //FIXME should add a timeout so the game eventually starts without
    //those who don't answer
    else
    {
        int someone_connected = 0;
        int someone_not_ready = 0;
        for(i = 0; i < MAX_CLIENTS; i++)
        {
            if(slave_thread[thread_id_no].client[i].sock != NULL)
            { 
                someone_connected = 1;
                if (!slave_thread[thread_id_no].client[i].game_ready)
                {
                    someone_not_ready = 1;
                }
            }
        }
        if(someone_connected && !someone_not_ready)
            start_game(thread_id_no); 
    }
}



void handle_client_nongame_msg(int thread_id_no, int i, char* buffer)
{
    DEBUGMSG(debug_lan, "nongame_msg received from client: %s\n", buffer);

    if(strncmp(buffer, "PLAYER_READY", strlen("PLAYER_READY")) == 0)
    {
        slave_thread[thread_id_no].client[i].game_ready = 1;
        //Inform other clients:
        send_player_updates(thread_id_no);
        //This will call start_game() if all the other clients are ready:
        check_game_clients(thread_id_no);
    }
    else if(strncmp(buffer, "PLAYER_NOT_READY", strlen("PLAYER_NOT_READY")) == 0)
    {
        slave_thread[thread_id_no].client[i].game_ready = 0;
        //Inform other clients:
        send_player_updates(thread_id_no);
        check_game_clients(thread_id_no);
    }
    else if(strncmp(buffer, "SET_NAME", strlen("SET_NAME")) == 0)
    {
        msg_set_name(thread_id_no, i, buffer);
    }
    else if(strncmp(buffer, "REQUEST_INDEX", strlen("REQUEST_INDEX")) == 0)
    {
        msg_socket_index(thread_id_no, i, buffer);
    }                            
}


int handle_client_game_msg(int thread_id_no, int i , char* buffer)
{
    DEBUGMSG(debug_lan, "game_msg received from client: %s\n", buffer);

    if(strncmp(buffer, "CORRECT_ANSWER", strlen("CORRECT_ANSWER")) == 0)
    {
        game_msg_correct_answer(thread_id_no,i, buffer);
    }                            
    else if(strncmp(buffer, "REQUEST_INDEX", strlen("REQUEST_INDEX")) == 0)
    {
        msg_socket_index(thread_id_no, i, buffer);
    }                            

    else if(strncmp(buffer, "WRONG_ANSWER",strlen("WRONG_ANSWER")) == 0) /* Player answered the question incorrectly , meaning comet crashed into a city or an igloo */
    {
        game_msg_wrong_answer(thread_id_no,i, buffer);
    }

    else if(strncmp(buffer, "LEAVE_GAME", strlen("LEAVE_GAME")) == 0) 
    {
        slave_thread[thread_id_no].client[i].game_ready = 0;  /* Player quitting game but not disconnecting */
    }

    else if(strncmp(buffer, "exit",strlen("exit")) == 0) /* Terminate this connection */
    {
        game_msg_exit(thread_id_no, i);
    }

    else if(strncmp(buffer, "quit",strlen("quit")) == 0) /* Quit the program */
    {
        game_msg_quit(thread_id_no, i);
        return(1);
    }
    else
    {
        fprintf(stderr, "command %s not recognized\n", buffer);
    }
    return(0);
}



int msg_set_name(int thread_id_no,int i, char* buf)
{
    char* p;

    if(buf == NULL)
        return 0;

    p = strchr(buf, '\t');
    if(p)
    { 
        p++;
        strncpy(slave_thread[thread_id_no].client[i].name, p, NAME_SIZE);
        send_player_updates(thread_id_no);
        return 1;
    }
    else
        return 0;
}


void msg_socket_index(int thread_id_no, int i, char* buf)
{  
    snprintf(buf, NET_BUF_LEN, "%s\t%d", "SOCKET_INDEX", i);
    SDLNet_TCP_Send(slave_thread[thread_id_no].client[i].sock, buf, NET_BUF_LEN);
}


void game_msg_correct_answer(int thread_id_no,int i, char* inbuf)
{
    char outbuf[NET_BUF_LEN];
    char* p = NULL;
    int id = -1;
    float t = -1;
    int points = 0;

    if(!inbuf)
        return;

    //parse inbuf to get question id:
    p = strchr(inbuf, '\t');
    if(!p)
        return; 
    p++;
    id = atoi(p);
    //Now get time player took to answer:
    p = strchr(p, '\t');
    if(!p)
        t = -1;
    else
    {
        p++;
        t = atof(p);
    }

    //Tell mathcards so lists get updated:
    points = MC_AnsweredCorrectly(lan_game_settings, id, t);
    if(!points)
        return;
    //If we get to here, the id was successfully parsed out of inbuf
    //and the corresponding question was found.
    slave_thread[thread_id_no].client[i].score += points;
    slave_thread[thread_id_no].srv_game.active_quests--;

    //Announcement for server and all clients:
    snprintf(outbuf, NET_BUF_LEN, 
            "question id %d was answered in %f seconds for %d points by %s",
            id, t, points, slave_thread[thread_id_no].client[i].name);             
    broadcast_msg(thread_id_no, outbuf);

    DEBUGMSG(debug_lan, "\ngame_msg_correct_answer(): %s\n", outbuf);
    DEBUGMSG(debug_lan, "After correct answer, wave %d\n"
            "srv_game.max_quests_on_screen = %d\n"
            "srv_game.rem_in_wave = %d\n"
            "srv_game.active_quests = %d\n\n",
            slave_thread[thread_id_no].srv_game.wave, slave_thread[thread_id_no].srv_game.max_quests_on_screen,
            slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests);   

    //Tell all players to remove that question:
    remove_question(thread_id_no, id, i);
    //and update the game counters:
    send_counter_updates(thread_id_no);
    //and the scores:
    send_player_updates(thread_id_no);
}


void game_msg_wrong_answer(int thread_id_no, int i, char* inbuf)
{
    char outbuf[NET_BUF_LEN];
    char* p;
    int id;

    if(!inbuf)
        return;

    //parse inbuf to get question id:
    p = strchr(inbuf, '\t');
    if(!p)
        return; 
    p++;
    id = atoi(p);

    //Tell mathcards so lists get updated:
    if(!MC_NotAnsweredCorrectly(lan_game_settings, id))
        return;
    //If we get to here, the id was successfully parsed out of inbuf
    //and the corresponding question was found.

    //One less comet in play:
    slave_thread[thread_id_no].srv_game.active_quests--;

    DEBUGMSG(debug_lan, "\nAfter wrong answer: wave %d\n"
            "srv_game.max_quests_on_screen = %d\n"
            "srv_game.rem_in_wave = %d\n"
            "srv_game.active_quests = %d\n\n",
            slave_thread[thread_id_no].srv_game.wave, slave_thread[thread_id_no].srv_game.max_quests_on_screen,
            slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests);   
    //Announcement for server and all clients:
    snprintf(outbuf, NET_BUF_LEN, 
            "question id %d was missed by %s\n",
            id, slave_thread[thread_id_no].client[i].name);             
    broadcast_msg(thread_id_no,outbuf);
    //Tell all players to remove that question:
    //-1 means question was missed.
    remove_question(thread_id_no, id, -1);
    //and update the game counters:
    send_counter_updates(thread_id_no);
}



void game_msg_next_question(int thread_id_no)
{
    MC_FlashCard flash;

    /* Get next question from MathCards: */
    if (!MC_NextQuestion(lan_game_settings, &flash))
    { 
        /* no more questions available */
        DEBUGMSG(debug_lan, "MC_NextQuestion() returned NULL - no questions available\n");
        return;
    }

    DEBUGMSG(debug_lan, "In game_msg_next_question(), about to send:\n");
    DEBUGCODE(debug_lan) print_card(flash); 

    /* Send it to all the clients: */ 
    add_question(thread_id_no, &flash);
    /* Adjust counters accordingly: */
    slave_thread[thread_id_no].srv_game.active_quests++;
    slave_thread[thread_id_no].srv_game.rem_in_wave--;

    DEBUGMSG(debug_lan, "In game_msg_next_question(), after quest added, wave %d\n"
            "srv_game.max_quests_on_screen = %d\n"
            "srv_game.rem_in_wave = %d\n"
            "srv_game.active_quests = %d\n\n",
            slave_thread[thread_id_no].srv_game.wave, slave_thread[thread_id_no].srv_game.max_quests_on_screen,
            slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests);   
}





void game_msg_exit(int thread_id_no, int i)
{
    fprintf(stderr, "LEFT the GAME : %s",slave_thread[thread_id_no].client[i].name);
    remove_client(thread_id_no, i);
}



//FIXME don't think we want to allow players to shut down the server
void game_msg_quit(int thread_id_no, int i)
{
    fprintf(stderr, "Server has been shut down by %s\n", slave_thread[thread_id_no].client[i].name); 
    cleanup_server(0);                 //FIXME Deepak its hard coded.  
    exit(9);                           // '9' means exit ;)  (just taken an arbitary no:)
}


/* Now this gets called to actually start the game once all the players */
/* have indicated that they are ready:                                  */
void start_game(int thread_id_no)
{
    char buf[NET_BUF_LEN];
    int j;


    /* NOTE this should no longer be needed - doing the same thing earlier    */
    /*This loop sees that the game starts only when all the players are ready */
    /* i.e. if someone is connected but not ready, we return.                 */
    for(j = 0; j < MAX_CLIENTS; j++)
    {
        // Only check sockets that aren't null:
        if((slave_thread[thread_id_no].client[j].game_ready != 1)
                && (slave_thread[thread_id_no].client[j].sock != NULL))
        {
            fprintf(stderr, "Warning - start_game() entered when someone not ready\n");
            return;      
        }
    }


    /***********************Will be modified**************/
    //Tell everyone we are starting and count who's really in:
    slave_thread[thread_id_no].num_clients = 0;
    snprintf(buf, NET_BUF_LEN, 
            "%s\n",
            "GO_TO_GAME");          
    for(j = 0; j < MAX_CLIENTS; j++)
    {
        if((slave_thread[thread_id_no].client[j].game_ready == 1)
                && (slave_thread[thread_id_no].client[j].sock != NULL))
        {
            if(SDLNet_TCP_Send(slave_thread[thread_id_no].client[j].sock, buf, NET_BUF_LEN) == NET_BUF_LEN)
                slave_thread[thread_id_no].num_clients++;
            else
            {
                fprintf(stderr, "in start_game() - failed to send to client %d, removing\n", j);
                remove_client(thread_id_no, j);
            }
        }
    }
    /*****************************************************/


    /* If no players join the game (should not happen) */
    if(slave_thread[thread_id_no].num_clients == 0)
    {
        fprintf(stderr, "There were no players........=(\n");
        return;
    }

    DEBUGMSG(debug_lan, "We have %d players.......\n", slave_thread[thread_id_no].num_clients);

    game_in_progress = 1;  //setting the game_in_progress flag to '1'
    //Start a new math game as far as mathcards is concerned:
    //TODO we could create more than one MathCards instance here when
    //we start supporting multiple simultaneous games in the future.  For now
    //we just use the lan_game_settings instance - DSB.
    if (!MC_StartGame(lan_game_settings))
    {
        fprintf(stderr, "\nMC_StartGame() failed!");
        return;
    }

    /* Initialize game data that isn't handled by mathcards: */
    slave_thread[thread_id_no].srv_game.wave = 1;
    slave_thread[thread_id_no].srv_game.active_quests = 0;
    slave_thread[thread_id_no].srv_game.max_quests_on_screen = Opts_StartingComets();
    slave_thread[thread_id_no].srv_game.quests_in_wave = slave_thread[thread_id_no].srv_game.rem_in_wave = Opts_StartingComets() * 2;

    game_in_progress = 1;

    // Zero out scores:
    for(j = 0; j < MAX_CLIENTS; j++)
        slave_thread[thread_id_no].client[j].score = 0;

    // Initialize game data:

    /* FIXME the queue is going away - need to time these so they don't
     * all go out simultaneously.
     */
    /* Send enough questions to fill the initial comet slots (currently 10) */
    //for(j = 0; j < QUEST_QUEUE_SIZE; j++)
    //{
    //  if (!MC_NextQuestion(&flash))
    //  { 
    //    /* no more questions available */
    //    fprintf(stderr, "MC_NextQuestion() returned NULL - no questions available\n");
    //    return;
    //  }

    //  DEBUGMSG(debug_lan, "In start_game(), about to send:\n");
    //  DEBUGCODE(debug_lan) print_card(flash); 

    //  //Send to all clients with add_question();
    //  add_question(&flash);
    //}

    //Send all the clients the counter totals:
    send_counter_updates(thread_id_no);
    send_player_updates(thread_id_no);
}

/* Update anything that isn't a response to a client message, such
 * as timer-based events:
 */
void server_update_game(int thread_id_no)
{
    static Uint32 last_time, now_time, wait_time;

    /* Do nothing unless game started: */
    if(!game_in_progress)
    {
        return;
    }

    now_time = SDL_GetTicks();

    /* Wait time is shorter in higher waves because the comets move faster: */
    wait_time = SRV_QUEST_INTERVAL/pow(DEFAULT_SPEEDUP_FACTOR, slave_thread[thread_id_no].srv_game.wave);

    /* Send another question if there is room and enough time has elapsed: */
    if(now_time - last_time > wait_time)
    {     
        if((slave_thread[thread_id_no].srv_game.active_quests < slave_thread[thread_id_no].srv_game.max_quests_on_screen)
                && (slave_thread[thread_id_no].srv_game.rem_in_wave > 0))
        {
            DEBUGMSG(debug_lan, "\nAbout to add next question:\n"
                    "srv_game.max_quests_on_screen = %d\n"
                    "srv_game.rem_in_wave = %d\n"
                    "srv_game.active_quests = %d\n"
                    "last_time = %d\n"
                    "now_time = %d\n\n",
                    slave_thread[thread_id_no].srv_game.max_quests_on_screen,
                    slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests,
                    last_time, now_time);   
            game_msg_next_question(thread_id_no);
            last_time = now_time;
        }
    }

    /* Go on to next wave when appropriate: */
    if(  slave_thread[thread_id_no].srv_game.rem_in_wave <= 0
            && slave_thread[thread_id_no].srv_game.active_quests <= 0)
    {
        slave_thread[thread_id_no].srv_game.wave++;
        slave_thread[thread_id_no].srv_game.active_quests = 0; 
        slave_thread[thread_id_no].srv_game.max_quests_on_screen += Opts_ExtraCometsPerWave(); 
        if(slave_thread[thread_id_no].srv_game.max_quests_on_screen > Opts_MaxComets()) 
            slave_thread[thread_id_no].srv_game.max_quests_on_screen = Opts_MaxComets(); 
        slave_thread[thread_id_no].srv_game.rem_in_wave = slave_thread[thread_id_no].srv_game.max_quests_on_screen * 2;
        send_counter_updates(thread_id_no); 
        DEBUGMSG(debug_lan, "/nAdvance to wave %d\n"
                "srv_game.max_quests_on_screen = %d\n"
                "srv_game.rem_in_wave = %d\n"
                "srv_game.active_quests = %d\n\n",
                slave_thread[thread_id_no].srv_game.wave, slave_thread[thread_id_no].srv_game.max_quests_on_screen,
                slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests);   

    }

    /* Find out from mathcards if we're done: */
    if(MC_TotalQuestionsLeft(lan_game_settings) == 0)
    {
        game_in_progress = 0;
        DEBUGMSG(debug_lan, "/nGame over:\nwave = %d\n"
                "srv_game.max_quests_on_screen = %d\n"
                "srv_game.rem_in_wave = %d\n"
                "srv_game.active_quests = %d\n\n",
                slave_thread[thread_id_no].srv_game.wave, slave_thread[thread_id_no].srv_game.max_quests_on_screen,
                slave_thread[thread_id_no].srv_game.rem_in_wave, slave_thread[thread_id_no].srv_game.active_quests);   

    }
}


/* Shut down game in progress: */
void end_game(int thread_id_no)
{
    int i = 0;
    char buf[NET_BUF_LEN];

    DEBUGMSG(debug_lan, "Enter end_game()\n");

    /* Broadcast notice to anyone who is left: */
    snprintf(buf, NET_BUF_LEN, "%s", "GAME_HALTED");
    transmit_all(thread_id_no,buf);

    /* Now make sure all clients are closed: */ 
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        SDLNet_TCP_Close(slave_thread[thread_id_no].client[i].sock);
        slave_thread[thread_id_no].client[i].sock = NULL;
        slave_thread[thread_id_no].client[i].game_ready = 0;
    }

    game_in_progress = 0;
    //  NOTE: we only want to call MC_EndGame() when the program exits,
    //  not when an individual math game ends.
    //  MC_EndGame();
    DEBUGMSG(debug_lan, "Leave end_game()\n");
}


//More centralized function to update the clients of the number of 
//questions remaining, whether the mission has been accomplished,
//and so forth:
int send_counter_updates(int thread_id_no)
{
    int total_questions;

    //If game won, tell everyone:
    if(MC_MissionAccomplished(lan_game_settings))
    {
        char buf[NET_BUF_LEN];
        snprintf(buf, NET_BUF_LEN, "%s", "MISSION_ACCOMPLISHED");
        transmit_all(thread_id_no, buf);
    }

    //Tell everyone how many questions left:
    total_questions = MC_TotalQuestionsLeft(lan_game_settings);
    {
        char buf[NET_BUF_LEN];
        snprintf(buf, NET_BUF_LEN, "%s\t%d", "TOTAL_QUESTIONS", total_questions);
        transmit_all(thread_id_no, buf);
    }

    //Tell everyone what wave we are on:
    {
        char buf[NET_BUF_LEN];
        snprintf(buf, NET_BUF_LEN, "%s\t%d", "WAVE", slave_thread[thread_id_no].srv_game.wave);
        transmit_all(thread_id_no, buf);
    }
    return 1;
}


int send_player_updates(int thread_id_no)
{
    int i = 0;

    /* Count how many players are active and send number to clients: */
    {
        int connected_players = 0;
        char buf[NET_BUF_LEN];
        for(i = 0; i < MAX_CLIENTS; i++)
            if((slave_thread[thread_id_no].client[i].game_ready == 1) && (slave_thread[thread_id_no].client[i].sock != NULL))
                connected_players++;

        snprintf(buf, NET_BUF_LEN, "%s\t%d", "CONNECTED_PLAYERS",
                connected_players);
        transmit_all(thread_id_no, buf);
    }

    /* Now send out all the names and scores: */
    for(i = 0; i < MAX_CLIENTS; i++)
    {
        if(slave_thread[thread_id_no].client[i].sock != NULL)
        {
            char buf[NET_BUF_LEN];
            snprintf(buf, NET_BUF_LEN, "%s\t%d\t%d\t%s\t%d", "UPDATE_PLAYER_INFO",
                    i,
                    slave_thread[thread_id_no].client[i].game_ready,
                    slave_thread[thread_id_no].client[i].name,
                    slave_thread[thread_id_no].client[i].score);
            transmit_all(thread_id_no, buf);
        }
    }

    return 1;
}


/* Sends a new question to all clients: */
int add_question(int thread_id_no, MC_FlashCard* fc)
{
    char buf[NET_BUF_LEN];

    if(!fc)
        return 0;

    snprintf(buf, NET_BUF_LEN,"%s\t%d\t%d\t%d\t%s\t%s\n",
            "ADD_QUESTION",
            fc->question_id,
            fc->difficulty,
            fc->answer,
            fc->answer_string,
            fc->formula_string);
    transmit_all(thread_id_no, buf);
    return 1;
}

/* Tells all clients to remove a specific question: */
int remove_question(int thread_id_no, int quest_id, int answered_by)
{
    char buf[NET_BUF_LEN];
    snprintf(buf, NET_BUF_LEN, "%s\t%d\t%d", "REMOVE_QUESTION", quest_id, answered_by);
    transmit_all(thread_id_no, buf);
    return 1;
}


/* Sends a string for the client to display to player: */
int player_msg(int thread_id_no, int i, char* msg)
{
    char buf[NET_BUF_LEN];
    if(!msg)
    {
        DEBUGMSG(debug_lan, "player_msg() - msg argument is NULL\n");
        return 0;
    }

    /* Add header: */
    snprintf(buf, NET_BUF_LEN, "%s\t%s", "PLAYER_MSG", msg);
    //NOTE transmit() validates index and socket
    return transmit(thread_id_no, i, buf);
}

/* Send a player message to all clients: */
void broadcast_msg(int thread_id_no, char* msg)
{
    int i = 0;
    if (!msg)
        return;
    for(i = 0; i < MAX_CLIENTS; i++)
        player_msg(thread_id_no, i, msg);
}

/* Send string to client. String should already have its header */ 
int transmit(int thread_id_no, int i, char* msg)
{
    char buf[NET_BUF_LEN];

    //Validate arguments;
    if(i < 0 || i > MAX_CLIENTS)
    {
        DEBUGMSG(debug_lan,"transmit() - invalid index argument\n");
        return 0;
    }

    if(!msg)
    {
        DEBUGMSG(debug_lan, "transmit() - msg argument is NULL\n");
        return 0;
    }

    if(!slave_thread[thread_id_no].client[i].sock)
    {
        return 0;
    }

    //NOTE SDLNet's Send() keeps sending until the requested length is
    //sent, so it really is an error if we send less thatn NET_BUF_LEN
    snprintf(buf, NET_BUF_LEN, "%s", msg);
    if(SDLNet_TCP_Send(slave_thread[thread_id_no].client[i].sock, buf, NET_BUF_LEN) < NET_BUF_LEN)
    {
        fprintf(stderr, "The client %s is disconnected\n", slave_thread[thread_id_no].client[i].name);
        remove_client(thread_id_no, i);
        return 0;
    }
    //Success:
    return 1;
}


/* Send the message to all clients: */
int transmit_all(int thread_id_no, char* msg)
{
    int i = 0;
    if (!msg)
        return 0;

    for(i = 0; i < MAX_CLIENTS; i++)
        transmit(thread_id_no, i, msg);

    return 1;
}



//Here we read up to max_length bytes from stdin into the buffer.
//The first '\n' in the buffer, if present, is replaced with a
//null terminator.
//returns 0 if no data ready, 1 if at least one byte read.
//NOTE for this to work we must first set stdin to O_NONBLOCK with:
//  fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) | O_NONBLOCK);

int read_stdin_nonblock(char* buf, size_t max_length)
{
#ifdef HAVE_FCNTL
    int bytes_read = 0;
    char* term = NULL;
    buf[0] = '\0';

    bytes_read = fread (buf, 1, max_length, stdin);
    term = strchr(buf, '\n');
    if (term)
        *term = '\0';

    if(bytes_read > 0)
        bytes_read = 1;
    else
        bytes_read = 0;

    return bytes_read;
#else
    return 0;
#endif
}




#endif
