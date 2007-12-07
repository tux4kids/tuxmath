/*
  tuxmath-admin.c

  Administer user tuxmath accounts: create accounts, clear gold stars, etc.

  by Tim Holy
  holy@wustl.edu

  Part of "Tux4Kids" Project
  http://tux4kids.alioth.debian.org/
  Subversion repository:
  https://svn.debian.alioth.org/tux4kids/tuxmath/

 
  December 3, 2007
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
// The next two are for mkdir and umask
#include <sys/types.h>
#include <sys/stat.h>
// The next is needed for opendir
#include <dirent.h>

#ifndef MACOSX
//#include "../config.h"
#endif

#define USER_MENU_ENTRIES "user_menu_entries"

#define PATH_MAX 4096
#define ADMINVERSION "0.1"

void display_help(void);
void usage(int err, char * cmd);
int extract_variable(FILE *fp, const char *varname, char** value);

void display_help(void)
{
  printf("\ntuxmath-admin\n"
	 "This program allows you to administer tuxmath, and is particularly\n"
	 "useful for schools and the like that may have many users.\n\n"
	 "Examples:\n"
	 "  tuxmath-admin --path /servervolume/tuxmath_users --createhomedirs users.csv\n"
	 "  tuxmath-admin --createhomedirs users.csv\n"
	 "    Creates a user directory tree in location /servervolume/tuxmath_users,\n"
	 "    according to the structure specified in users.csv.  See configure.pdf\n"
	 "    for details.  The second syntax is applicable if you've defined the\n"
	 "    homedir path in the global configuration file.\n\n"
	 "  tuxmath-admin --confighighscores --level 2\n"
	 "    Sets up sharing of high scores at level 2 of the hierarchy (top is\n"
	 "    level 1).  If you've divided things as School:Grade:Classroom:User, then\n"
	 "    this would correspond to sharing high scores among all kids in the same\n"
	 "    grade.\n\n"
	 "  tuxmath-admin --clearhighscores\n"
	 "    Clears high scores for all users in the location specified by the homedir\n"
	 "    setting in the global configuration file.\n\n"
	 "  tuxmath-admin --path /servervolume/tuxmath_users/2ndgrade --clearhighscores\n"
	 "    Clears the high scores for all users inside the 2ndgrade hierarchy.\n\n"
	 "  tuxmath-admin --cleargoldstars\n"
	 "    Clears the gold stars for all users.\n\n"
	 "  tuxmath-admin --path /servervolume/tuxmath_users/1st\\ grade/Mrs.\\ Smith --cleargoldstars\n"
	 "    Clears the gold stars for all users in Mrs. Smith's first grade class.\n"
	 );
}

// Extracts a single variable from a configuration file. Returns 1
// on success and 0 on failure.
int extract_variable(FILE *fp, const char *varname, char** value)
{
  char buf[PATH_MAX];
  char *param_begin;
  char *tmpvalue;

  rewind(fp);  // start at the beginning of the file

  // Read in a line at a time:
  while (fgets (buf, PATH_MAX, fp)) {
    param_begin = buf;
    // Skip leading whitespace
    while (isspace(*param_begin))
      param_begin++;
    // Skip comments
    if ((*param_begin == ';') || (*param_begin == '#'))
      continue;
    // Test whether it matches the variable name
    if (strncmp(param_begin,varname,strlen(varname)) == 0) {
      // Find the "=" sign
      tmpvalue = strchr(param_begin+strlen(varname), '=');
      if (tmpvalue == NULL)
	continue;
      // Skip whitespace
      while (isspace(*tmpvalue))
	tmpvalue++;
      // Copy the result
      *value = strdup(tmpvalue);
      return 1;
    }
  }
  return 0;
}

void create_homedirs(const char *path,const char *file)
{
  FILE *fp,*fpue;
  char buf[PATH_MAX];
  char *line_begin;
  char *line_cur;
  char *line_cur_end;
  char *copy_start;
  char fullpath[PATH_MAX];
  char **current_dirtree = NULL;
  int current_depth;
  int old_depth = -1;
  int max_depth = 0;
  int this_line_total_depth;
  int stop_blanking;
  int i;
  int len;
  mode_t mask;

  fp = fopen(file,"r");
  if (!fp) {
    fprintf(stderr,"Error: couldn't open %s for reading.\n",file);
    exit(EXIT_FAILURE);
  }
  mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IXUSR | S_IXGRP | S_IXOTH;
  umask(0x0);  // make dirs read/write for everyone
  while (fgets (buf, PATH_MAX, fp)) {
    line_begin = buf;
    // Skip leading whitespace
    while (isspace(*line_begin))
      line_begin++;
    // Skip comments
    if ((*line_begin == ';') || (*line_begin == '#'))
      continue;
    // Make sure this line isn't blank
    if (strlen(line_begin) == 0)
      continue;
    //printf("Read the line %s\n",line_begin);

    // Count the number of levels by counting the commas + 1
    this_line_total_depth = 1;
    line_cur = line_begin;
    while (!(*line_cur == '\r' || *line_cur == '\n')) {
      if (*line_cur == ',')
	this_line_total_depth++;
      line_cur++;
    }

    // If this is our first time, set up the tree structure
    if (max_depth == 0) {
      max_depth = this_line_total_depth;
      current_dirtree = (char **) malloc(max_depth * sizeof(char*));
      if (current_dirtree == NULL) {
	fprintf(stderr,"Error: couldn't allocate memory for directory tree.\n");
	exit(EXIT_FAILURE);
      }
      for (i = 0; i < max_depth; i++) {
	current_dirtree[i] = (char *) malloc(PATH_MAX * sizeof(char));
	if (current_dirtree[i] == NULL){
	  fprintf(stderr,"Error: couldn't allocate memory for directory tree.\n");
	  exit(EXIT_FAILURE);
	} else
	  *(current_dirtree[i]) = '\0';  // initialize with blank string
      }
    }
    else {
      // Check that this line doesn't change the size of the directory hierarchy
      if (this_line_total_depth != max_depth) {
	fprintf(stderr,"Error: line\n  '%s'\ncontains a different number of depths to the hierarchy than the previous setting (%d).\n",buf,max_depth);
	exit(EXIT_FAILURE);
      }
    }
    
    // Parse the directories from back to front.  Blank fields at the
    // end indicate a lack of subdirectories; blank fields at the
    // beginning indicate that the higher levels of the hierarchy are
    // not to be changed.  So these have to be treated differently.
    *line_cur = '\0';  // replace linefeed with terminal \0
    line_cur_end = line_cur;
    current_depth = max_depth-1;
    stop_blanking = 0;
    while (current_depth >= 0) {
      // Back up to the previous comma
      // Note that line_cur+1 points to the first "real character" of
      // the string, so don't be bothered that line_cur could get to be
      // one less than line_begin.
      while (line_cur >= line_begin && *line_cur != ',')
	line_cur--;
      // Determine whether we have a new directory name
      if (line_cur+1 < line_cur_end) {
	// We do, copy it over including the terminal \0
	copy_start = line_cur+1;
	if (*copy_start == '\"')
	  copy_start++;
	if (line_cur_end[-1] == '\"') {
	  line_cur_end--;
	  *line_cur_end = '\0';
	}
	memcpy(current_dirtree[current_depth],copy_start,line_cur_end-copy_start+1);
	stop_blanking = 1;  // don't clear blank fields in the future
      }
      else {
	// Blank this particular field, because we don't want old
	// subdirectories hanging around
	if (!stop_blanking)
	  *(current_dirtree[current_depth]) = '\0';
      }
      current_depth--;
      if (line_cur >= line_begin)
	*line_cur = '\0'; // end the processing at the comma
      line_cur_end = line_cur;
    }

    // Create the full path
    strncpy(fullpath, path, PATH_MAX);
    len = strlen(fullpath);
    if (fullpath[len-1] != '/' && len+1 < PATH_MAX) {
      fullpath[len] = '/';  // append a slash, if need be
      fullpath[len+1] = '\0';
    }
    for (i = 0; i < max_depth; i++) {
      len = strlen(fullpath);
      strncpy(fullpath+len,current_dirtree[i],PATH_MAX-len);
      len = strlen(fullpath);
      if (fullpath[len-1] != '/' && len+1 < PATH_MAX) {
	fullpath[len] = '/';  // append a slash, if need be
	fullpath[len+1] = '\0';
      }
    }

    // Create the directory
    if (strlen(fullpath) < PATH_MAX) {
      if (mkdir(fullpath,mask) < 0) {
	// There was some kind of error, figure out what happened
	if (errno == EEXIST) {
	  fprintf(stderr,"Warning: %s already exists, continuing.\n",fullpath);
	}
	else if (errno == ENAMETOOLONG) {
	  fprintf(stderr,"Error: the directory name:\n  %s\nwas too long.\n",fullpath);
	  exit(EXIT_FAILURE);
	}
	else if (errno == ENOENT) {
	  fprintf(stderr,"Error: One of the upper-level directories in:\n  %s\ndoesn't exist.  Check the syntax of your configuration file.\n",fullpath);
	  exit(EXIT_FAILURE);
	}
	else if (errno == ENOSPC) {
	  fprintf(stderr,"Error: the device has no room available.\n");
	  exit(EXIT_FAILURE);
	}
	else {
	  // This includes EACCESS and all other errors
	  fprintf(stderr,"Error: couldn't make directory %s:\nDo you have write permission for this location?\nDo you need to be root/administrator?\n",fullpath);
	  error(1,errno,"error");
	}
      }
      else {
	fsync(stderr);
	fprintf(stdout,"Creating %s\n",fullpath);
	fsync(stdout);

	// Append the name to the user_menu_entries file
	// First we split off the last item in fullpath
	line_begin = fullpath;
	len = strlen(line_begin);
	line_begin[len-1] = '\0';  // replace terminal '/' with \0
	line_cur = line_begin + len-1;
	while (line_cur > line_begin && *line_cur != '/')
	  line_cur--;
	if (line_cur > line_begin) { // as long as not making in the root directory...a bad idea anyway!
	  *line_cur = '\0';  // Split into two strings
	}
	else {
	  line_begin = "/";
	}
	line_cur++;   // line_cur now points to beginning of newest directory
	strncpy(buf,line_begin,PATH_MAX);  // we don't need buf anymore
	buf[strlen(buf)] = '/';  // append directory separator
	len = strlen(buf);
	strncpy(buf+len,USER_MENU_ENTRIES,PATH_MAX-len-strlen(USER_MENU_ENTRIES));
	// Now do the appending
	fpue = fopen(buf,"a");
	if (!fpue) {
	  fprintf(stderr,"Error: can't open file %s for writing.\n",buf);
	  exit(EXIT_FAILURE);
	}
	len = fprintf(fpue,"%s\n",line_cur);
	if (len != strlen(line_cur)+1) {
	  fprintf(stderr,"Error writing %s to file %s.\n",line_cur,buf);
	  error(1,error,"");
	}
	fclose(fpue);
      }
    }
    else {
      // The path name was truncated, don't make a corrupt directory
      fprintf(stderr,"Error: the directory name:\n  %s\nwas too long, quitting.\n",fullpath);
      exit(EXIT_FAILURE);
    }
    //printf("Directory: %s\n",fullpath);
  }
  
  // Free memory
  for (i = 0; i < max_depth; i++)
    free(current_dirtree[i]);
  if (current_dirtree != NULL)
    free(current_dirtree);
}

int main(int argc, char *argv[])
{
  int i;
  FILE *fp;
  DIR *dir;

  int is_creatinghomedirs = 0;
  int is_confighighscores = 0;
  int is_clearinggoldstars = 0;
  int is_clearinghighscores = 0;
  char *path = NULL;
  char *file = NULL;
  int level = 0;
  int success;

  if (argc < 2) {
    display_help();
    exit(EXIT_FAILURE);
  }

  // Check global config file for a homedir path (must be uncommented)
  fp = fopen(DATA_PREFIX "/missions/options", "r");
  if (fp) {
    extract_variable(fp,"homedir",&path);
    fclose(fp);
  }

  // Parse the command line options
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      display_help();
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[i], "--copyright") == 0 ||
	     strcmp(argv[i], "-c") == 0)
    {
      printf(
	"\ntuxmath-admin version " ADMINVERSION ", Copyright (C) 2007 Tim Holy\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation.  See COPYING.txt\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	"\n");
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[i], "--usage") == 0 ||
	     strcmp(argv[i], "-u") == 0) {
      usage(0, argv[0]);
      exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[i], "--path") == 0) {
      if (i+1 > argc) {
	fprintf(stderr, "%s option requires an argument (a directory name)\n", argv[i]);
	usage(1, argv[0]);
      }
      else {
	path = argv[i+1];
	dir = opendir(path);  // determine whether directory exists
	if (dir == NULL) {
	  fprintf(stderr,"Error: path:\n  %s\nis not an existing directory, or could not be read.\n",path);
	  exit(EXIT_FAILURE);
	}
	closedir(dir);
	i++; // increment so further processing skips over the argument
      }
    }
    else if (strcmp(argv[i], "--level") == 0) {
      if (i+1 > argc) {
	fprintf(stderr, "%s option requires an argument (a level number)\n", argv[i]);
	usage(1, argv[0]);
      }
      else {
	success = sscanf(argv[i+1],"%d",&level);
	if (!success) {
	  fprintf(stderr,"level: %s is not a number\n",argv[i+1]);
	  exit(EXIT_FAILURE);
	}
      }
    }
    else if (strcmp(argv[i], "--createhomedirs") == 0) {
      is_creatinghomedirs = 1;
      if (i+1 > argc) {
	fprintf(stderr, "%s option requires an argument (a file name)\n", argv[i]);
	usage(1, argv[0]);
      }
      else {
	file = argv[i+1];
	fp = fopen(file,"r");   // determine whether the file exists
	if (fp == NULL) {
	  fprintf(stderr,"createhomedirs: %s is not an existing filename, or could not be read\n",file);
	  exit(EXIT_FAILURE);
	}
	fclose(fp);  // don't read it yet
	i++; // increment so further processing skips over the argument
      }
    }
    else if (strcmp(argv[i], "--confighighscores") == 0) {
      is_confighighscores = 1;
    }
    else if (strcmp(argv[i], "--clearinghighscores") == 0) {
      is_clearinghighscores = 1;
    }
    else if (strcmp(argv[i], "--clearinggoldstars") == 0) {
      is_clearinggoldstars = 1;
    }
    else {
      fprintf(stderr,"Error: option %s not recognized.\n",argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  // All operations require a valid path, so check that now
  if (path == NULL) {
    fprintf(stderr,"Must have a valid path (either with --path or in the global configuration)\n");
    usage(1, argv[0]);
    exit(EXIT_FAILURE);
  }

  // Create homedirs
  if (is_creatinghomedirs) {
    if (file == NULL) {
      fprintf(stderr,"Must specify a filename when creating homedirs\n");
      usage(1, argv[0]);
      exit(EXIT_FAILURE);
    }
    create_homedirs(path,file);
  }

  // Configure high scores
  if (is_confighighscores) {
    if (level == 0) {
      fprintf(stderr,"Must specify a level when configuring highscores\n");
      usage(1, argv[0]);
      exit(EXIT_FAILURE);
    }
    //config_highscores(path,level);
  }

  // Clear high scores
  if (is_clearinghighscores) {
    //clear_highscores(path);
  }

  // Clear gold stars
  if (is_clearinggoldstars) {
    //clear_goldstars(path);
  }
   
  return EXIT_SUCCESS;
}


void usage(int err, char * cmd)
{
  FILE * f;

  if (err == 0)
    f = stdout;
  else
    f = stderr;

  fprintf(f,
   "\nUsage: %s {--help | --usage | --copyright}\n"
   "       %s [--path <directory>] --createhomedirs <file>\n"
   "       %s [--level <levelnum>] --confighighscores\n"
   "       %s [--path <directory>] [--clearhighscores] [--cleargoldstars]\n"
    "\n", cmd, cmd, cmd, cmd);

  exit (err);
}

