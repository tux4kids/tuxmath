/*
C Interface: config.h

Description: 


Author: David Bruce <dbruce@tampabay.rr.com>, (C) 2006

Copyright: See COPYING file that comes with this distribution (briefly, GNU GPL)
*/

#ifndef CONFIG_H
#define CONFIG_H

#define PATH_MAX 4096

enum {
  USER_CONFIG_FILE,
  GLOBAL_CONFIG_FILE
};

/* FIXME these should return errors if they fail. */
int read_config_file(FILE* fp, int file_type);
int write_config_file(FILE* fp);

#endif
