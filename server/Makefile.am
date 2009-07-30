## Makefile.am for tuxmath server:
## Process with Automake to create Makefile.in

bin_PROGRAMS = tuxmathserver tuxmathtestclient

tuxmathserver_SOURCES = server.c \
	../src/mathcards.c	\
	 ../src/throttle.c

tuxmathtestclient_SOURCES = testclient.c \
                            ../src/throttle.c \
                            ../src/network.c

EXTRA_DIST = server.h testclient.h
 
