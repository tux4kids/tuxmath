/*
   tuxmathadmin.c:

   Administer user tuxmath accounts: create accounts, clear gold stars, etc.

   Copyright 2007, 2010, 2011.
Authors: Tim Holy
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

tuxmathadmin.c is part of "Tux, of Math Command", a.k.a. "tuxmath".

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



#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// error.h not always available - e.g. MacOSX, BeOS:
#ifdef HAVE_ERROR_H
#include <error.h>
#else
# ifndef HAVE_FSYNC
#  define fsync
# endif
# define error(status, errnum, rest...) \
    fsync(stdout); fprintf(stderr, ## rest); exit(status)
#endif

// The next two are for mkdir and umask
#include <sys/types.h>
#include <sys/stat.h>
// The next is needed for opendir
#include <dirent.h>
// The next is for isspace
#include <ctype.h>
// The next is for fsync
#include <unistd.h>


#ifdef BUILD_MINGW32
#define USER_MENU_ENTRIES_FILENAME "user_menu_entries.txt"
#define HIGHSCORE_FILENAME "highscores.txt"
#define GOLDSTAR_FILENAME "goldstars.txt"
#else
#define USER_MENU_ENTRIES_FILENAME "user_menu_entries"
#define HIGHSCORE_FILENAME "highscores"
#define GOLDSTAR_FILENAME "goldstars"
#endif

#define PATH_MAX 4096
#define MAX_USERS 100000
#define ADMINVERSION "0.1.1"

void display_help(void);
void usage(int err, char * cmd);
int extract_variable(FILE *fp, const char *varname, char** value);
int directory_crawl(const char *path);
void free_directories(int n);
void create_homedirs(const char *path,const char *file);
void config_highscores(const char *path,int level);
void unconfig_highscores(const char *path);
void clear_highscores(const char *path);
void clear_goldstars(const char *path);
void consolidate_logs(const char *path);
void clear_logs(const char *path);
void clear_file(const char *path,const char *filename,const char *invoke_name);
char* eatwhite(char *buf);

char *directory[MAX_USERS];
int directory_level[MAX_USERS];

int main(int argc, char *argv[])
{
    int i;
    FILE *fp;
    DIR *dir;

    int is_creatinghomedirs = 0;
    int is_confighighscores = 0;
    int is_unconfighighscores = 0;
    int is_clearinggoldstars = 0;
    int is_clearinghighscores = 0;
    int is_consolidatinglogs = 0;
    int is_clearinglogs = 0;
    char *path = NULL;
    char *file = NULL;
    int level = 0;
    int success;

    // Null-out global directory pointers
    for (i = 0; i < MAX_USERS; i++)
        directory[i] = NULL;

    if (argc < 2) {
        display_help();
        exit(EXIT_FAILURE);
    }

    // Check global config file for a homedir path (must be uncommented)
    fp = fopen(DATA_PREFIX "/missions/options", "r");
    if (fp) {
        extract_variable(fp,"homedir",&path);
        free(path);
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
                    "\ntuxmathadmin version " ADMINVERSION ", Copyright (C) 2007 Tim Holy\n"
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
                usage(EXIT_FAILURE, argv[0]);
            }
            else {
                path = argv[i+1];
                dir = opendir(path);  // determine whether directory exists
                if (dir == NULL)
                    error(EXIT_FAILURE,errno,"path:\n  %s",path);
                closedir(dir);
                i++; // increment so further processing skips over the argument
            }
        }
        else if (strcmp(argv[i], "--level") == 0) {
            if (i+1 > argc) {
                fprintf(stderr, "%s option requires an argument (a level number)\n", argv[i]);
                usage(EXIT_FAILURE, argv[0]);
            }
            else {
                success = sscanf(argv[i+1],"%d",&level);
                if (!success) {
                    fprintf(stderr,"level: %s is not a number\n",argv[i+1]);
                    exit(EXIT_FAILURE);
                }
                i++; // increment so further processing skips over the argument
            }
        }
        else if (strcmp(argv[i], "--createhomedirs") == 0) {
            is_creatinghomedirs = 1;
            if (i+1 > argc) {
                fprintf(stderr, "%s option requires an argument (a file name)\n", argv[i]);
                usage(EXIT_FAILURE, argv[0]);
            }
            else {
                file = argv[i+1];
                fp = fopen(file,"r");   // determine whether the file exists
                if (fp == NULL)
                    error(EXIT_FAILURE,errno,"createhomedirs using:\n  %s",file);
                fclose(fp);  // don't read it yet, do that elsewhere
                i++; // increment so further processing skips over the argument
            }
        }
        else if (strcmp(argv[i], "--confighighscores") == 0) {
            is_confighighscores = 1;
        }
        else if (strcmp(argv[i], "--unconfighighscores") == 0) {
            is_unconfighighscores = 1;
        }
        else if (strcmp(argv[i], "--clearhighscores") == 0) {
            is_clearinghighscores = 1;
        }
        else if (strcmp(argv[i], "--cleargoldstars") == 0) {
            is_clearinggoldstars = 1;
        }
        else if (strcmp(argv[i], "--consolidatelogs") == 0) {
            is_consolidatinglogs = 1;
        }
        else if (strcmp(argv[i], "--clearlogs") == 0) {
            is_clearinglogs = 1;
        }
        else {
            fprintf(stderr,"Error: option %s not recognized.\n",argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    // All operations require a valid path, so check that now
    if (path == NULL) {
        fprintf(stderr,"Must have a valid path (either with --path or in the global configuration)\n");
        usage(EXIT_FAILURE, argv[0]);
    }

    // Create homedirs
    if (is_creatinghomedirs) {
        if (file == NULL) {
            fprintf(stderr,"Must specify a filename when creating homedirs\n");
            usage(EXIT_FAILURE, argv[0]);
        }
        create_homedirs(path,file);
    }

    // Configure high scores
    if (is_confighighscores) {
        if (level == 0) {
            fprintf(stderr,"Must specify a level when configuring highscores\n");
            usage(EXIT_FAILURE, argv[0]);
        }
        config_highscores(path,level);
    }

    // Unconfigure high scores
    if (is_unconfighighscores) {
        unconfig_highscores(path);
    }

    // Clear high scores
    if (is_clearinghighscores) {
        clear_highscores(path);
    }

    // Clear gold stars
    if (is_clearinggoldstars) {
        clear_goldstars(path);
    }

    // Consolidate logs
    if (is_consolidatinglogs) {
        consolidate_logs(path);
    }

    // Clear logs
    if (is_clearinglogs) {
        clear_logs(path);
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
            "       %s [--path <directory>] [--clearhighscores] [--cleargoldstars] [--consolidatelogs] [--clearlogs]\n"
            "\n", cmd, cmd, cmd, cmd);

    exit (err);
}

void display_help(void)
{
    fprintf(stderr, "\ntuxmathadmin\n"
            "This program facilitates administering tuxmath, and is particularly\n"
            "useful for schools and the like that may have many users.\n\n"
            "Examples:\n"
            "  tuxmathadmin --path /servervolume/tuxmath_users --createhomedirs users.csv\n"
            "  tuxmathadmin --createhomedirs users.csv\n"
            "    Creates a user directory tree in location /servervolume/tuxmath_users,\n"
            "    according to the structure specified in users.csv.  See configure.pdf\n"
            "    for details.  The second syntax is applicable if you've defined the\n"
            "    homedir path in the global configuration file.\n\n"
            "  tuxmathadmin --confighighscores --level 3\n"
            "    Sets up sharing of high scores at level 3 of the hierarchy (top is\n"
            "    level 1).  If students logging in are presented with a choice of grade,\n"
            "    then classroom, and then user, then level 1 is the school, level 2 is the\n"
            "    grade, level 3 is the classroom, and level 4 is the individual student.\n"
            "    So level 3 would set it up so that all kids in the same classroom would\n"
            "    compete for high scores.\n\n"
            "  tuxmathadmin --unconfighighscores\n"
            "    Removes any existing highscores configuration.\n\n"
            "  tuxmathadmin --clearhighscores\n"
            "    Clears high scores for all users in the location specified by the homedir\n"
            "    setting in the global configuration file.\n\n"
            "  tuxmathadmin --path /servervolume/tuxmath_users/2ndgrade --clearhighscores\n"
            "    Clears the high scores for all users inside the 2ndgrade hierarchy.\n\n"
            "  tuxmathadmin --cleargoldstars\n"
            "    Clears the gold stars for all users.\n\n"
            "  tuxmathadmin --path /servervolume/tuxmath_users/1st\\ grade/Mrs.\\ Smith --cleargoldstars\n"
            "    Clears the gold stars for all users in Mrs. Smith's first grade class.\n\n"
            "  tuxmathadmin --consolidatelogs\n"
            "    Creates consolidated_log.csv files at one level above the lowest level\n"
            "    of the directory hierarchy. These files can be opened with a spreadsheet\n"
            "    program. This may be useful for tracking student progress.\n"
            "    Note also that each student has a personal log.csv file in his/her own\n"
            "    directory.\n\n"
            "  tuxmathadmin --clearlogs\n"
            "    Deletes all log.csv files in the directory hierarchy.\n\n"
            );
}

// This function does the work of creating the user directory tree,
// given the structure specified in the CSV (comma separated value)
// file "file".  "path" is the base directory in which this tree is
// created.
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
    int max_depth = 0;
    int this_line_total_depth;
    int stop_blanking;
    int i;
    int len;
    mode_t mask;

    fp = fopen(file,"r");
    if (!fp)
        error(EXIT_FAILURE,errno,"Error: couldn't open:\n  %s for reading",file);
    umask(0x0);  // make dirs read/write for everyone
    while (fgets (buf, PATH_MAX, fp)) {
        // Skip leading & trailing whitespace
        line_begin = eatwhite(buf);
        // Skip comments
        if ((*line_begin == ';') || (*line_begin == '#'))
            continue;
        // Make sure this line isn't blank
        if (strlen(line_begin) == 0)
            continue;
        //fprintf(stderr, "Read the line %s\n",line_begin);

        // Count the number of levels by counting the commas + 1
        this_line_total_depth = 1;
        line_cur = line_begin;
        while (!(*line_cur == '\r' || *line_cur == '\n' || *line_cur == '\0')) {
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

        // Parse the pathname from back to front.  Blank fields at the end
        // indicate a lack of subdirectories; blank fields at the
        // beginning indicate that the higher levels of the hierarchy are
        // not to be changed (just copied "down").  So these have to be
        // treated differently.
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
#ifndef BUILD_MINGW32
            mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IXUSR | S_IXGRP | S_IXOTH;
            if (mkdir(fullpath,mask) < 0) {
#else
                if (mkdir(fullpath) < 0) {
#endif

                    // There was some kind of error, figure out what happened.
                    // Be a little more verbose than the standard library errors.
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
                        // Fall back on the standard library for the remaining error
                        // handling
                        fprintf(stderr,"Error: couldn't make directory %s:\nDo you have write permission for this location?\nDo you need to be root/administrator?\n",fullpath);
                        error(EXIT_FAILURE,errno,"error");
                    }
                }
                else {
                    fsync(fileno(stderr));
                    fprintf(stdout,"Creating %s\n",fullpath);
                    fsync(fileno(stdout));

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
                    strncpy(buf+len,USER_MENU_ENTRIES_FILENAME,PATH_MAX-len-strlen(USER_MENU_ENTRIES_FILENAME));
                    // Now do the appending
                    fpue = fopen(buf,"a");
                    if (!fpue) {
                        fprintf(stderr,"Error: can't open file %s for writing.\n",buf);
                        exit(EXIT_FAILURE);
                    }
                    len = fprintf(fpue,"%s\n",line_cur);
                    if (len != strlen(line_cur)+1) {
                        error(EXIT_FAILURE,errno,"Error writing %s to file %s.\n",line_cur,buf);
                    }
                    fclose(fpue);
                }
            }
            else {
                // The path name was truncated, don't make a corrupt directory
                fprintf(stderr,"Error: the directory name:\n  %s\nwas too long, quitting.\n",fullpath);
                exit(EXIT_FAILURE);
            }
        }

        // Free memory
        for (i = 0; i < max_depth; i++)
            free(current_dirtree[i]);
        if (current_dirtree != NULL)
            free(current_dirtree);
    }


    // Creates blank highscores files at the specified level of the
    // directory hierarchy.  This will be the level at which highscore
    // competition will occur.
    void config_highscores(const char *path,int level)
    {
        FILE *fp;
        char buf[PATH_MAX];
        int n_dirs;
        int i;
        int success;

        n_dirs = directory_crawl(path);
        success = 0;  // This will change to 1 if we find a directory of the
        // right level
        for (i = 0; i < n_dirs; i++) {
            if (directory_level[i] == level) {
                // Create a blank highscores file in this directory
                strncpy(buf,directory[i],PATH_MAX);
                strncat(buf,HIGHSCORE_FILENAME,PATH_MAX-strlen(buf)-1);
                if (strlen(buf) >= PATH_MAX-1) {
                    fprintf(stderr,"confighighscores: pathname %s truncated, exiting.\n",buf);
                    exit(EXIT_FAILURE);
                }
                fp = fopen(buf,"w");
                if (!fp)
                    error(EXIT_FAILURE,errno,"confighighscores: file:\n  %s",buf);
                // That creates a blank file, which is all we have to do
                fclose(fp);
                success = 1;
            }
        }
        if (!success) {
            fprintf(stderr,"Error: no directories of level %d found!",level);
            exit(EXIT_FAILURE);
        }

        free_directories(n_dirs);
    }

    // Delete all highscores files in the directory hierarchy
    void unconfig_highscores(const char *path)
    {
        clear_file(path,HIGHSCORE_FILENAME,"unconfighighscores");
    }

    // Replaces all highscores files with blank files anywhere in the
    // directory hierarchy.  Replacing it with a blank file, rather than
    // just deleting it, insures that the highscores configuration (in
    // terms of the level at which highscore competition occurs) is not
    // altered.
    void clear_highscores(const char *path)
    {
        FILE *fp;
        char buf[PATH_MAX];
        int n_dirs;
        int i;

        n_dirs = directory_crawl(path);
        for (i = 0; i < n_dirs; i++) {
            // Search for a highscores file in this directory
            strncpy(buf,directory[i],PATH_MAX);
            strncat(buf,HIGHSCORE_FILENAME,PATH_MAX-strlen(buf)-1);
            if (strlen(buf) >= PATH_MAX-1) {
                fprintf(stderr,"clearhighscores: pathname %s truncated, exiting.\n",buf);
                exit(EXIT_FAILURE);
            }
            fp = fopen(buf,"r");
            if (fp) {
                // We found such a file, replace it with a blank one
                fclose(fp);
                fp = fopen(buf,"w");
                if (!fp)
                    error(EXIT_FAILURE,errno,"clearhighscores: file:\n  %s",buf);
                // That creates a blank file, which is all we have to do
                fclose(fp);
            }
        }

        free_directories(n_dirs);
    }

    // Delete all goldstars files in the directory hierarchy
    void clear_goldstars(const char *path)
    {
        clear_file(path,GOLDSTAR_FILENAME,"cleargoldstars");
    }

    // Create consolidated log files at the next-to-lowest level of the
    // hierarchy.  This is basically performing a "cat" operation on all
    // the log.csv files that are below a given point in a directory
    // hierarchy.
    void consolidate_logs(const char *path)
    {
        int n_dirs;
        int max_level;
        int i;
        int column_names_written = 0;
        FILE *fplogwrite, *fplogread, *fpusersread;
        char buf[PATH_MAX], buf2[PATH_MAX], buf3[PATH_MAX];
        char *line_begin;

        // Determine the maximum level
        n_dirs = directory_crawl(path);
        max_level = 0;
        for (i = 0; i < n_dirs; i++)
            if (directory_level[i] > max_level)
                max_level = directory_level[i];

        // For every directory on the next-to-maximum level,...
        for (i = 0; i < n_dirs; i++) {
            if (directory_level[i] != max_level-1)
                continue;

            // Open the user_menu_entries file, so that we can cycle through
            // all subdirectories
            strncpy(buf,directory[i],PATH_MAX);
            strncat(buf,USER_MENU_ENTRIES_FILENAME,PATH_MAX-strlen(buf)-1);
            if (strlen(buf) >= PATH_MAX-1) {
                error(EXIT_FAILURE,0,"consolidatelogs: pathname %s is truncated, exiting.\n",buf);
            }
            fpusersread = fopen(buf,"r");
            if (!fpusersread)
                error(EXIT_FAILURE,errno,"consolidatelogs: file:\n %s",buf);

            // Create a blank consolidated_log.csv file
            strncpy(buf,directory[i],PATH_MAX);
            strncat(buf,"consolidated_log.csv",PATH_MAX-strlen(buf)-1);
            if (strlen(buf) >= PATH_MAX-1) {
                error(EXIT_FAILURE,0,"consolidatelogs: pathname %s is truncated, exiting.\n",buf);
            }
            fplogwrite = fopen(buf,"w");
            if (!fplogwrite)
                error(EXIT_FAILURE,errno,"consolidatelogs: file:\n  %s",buf);

            // Loop over different users
            while (fgets(buf, PATH_MAX, fpusersread)) {
                // Skip over white space, and especially blank lines
                line_begin = eatwhite(buf);
                if (strlen(line_begin) == 0)
                    continue;
                // Create the full path & filename of the user's log.csv file
                strncpy(buf2,directory[i],PATH_MAX);
                strncat(buf2,line_begin,PATH_MAX-strlen(buf2)-1);
                strncat(buf2,"/log.csv",PATH_MAX-strlen(buf2)-1);
                fplogread = fopen(buf2,"r");
                if (fplogread) {
                    // Copy the relevant lines from the user's log.csv file to the
                    // consolidated log file.  Make sure only one copy of the
                    // column names is written.
                    while(fgets(buf3, PATH_MAX, fplogread)) {
                        line_begin = eatwhite(buf3);
                        if (strlen(line_begin) == 0)
                            continue;
                        if (strncmp(line_begin,"\"User",5) == 0) {
                            if (!column_names_written) {
                                fprintf(fplogwrite,"%s\n",line_begin);
                                column_names_written = 1;
                            }
                        } else {
                            fprintf(fplogwrite,"%s\n",line_begin);
                        }
                    }
                    fclose(fplogread);
                }
            }
            fclose(fpusersread);
            fclose(fplogwrite);
        }

        free_directories(n_dirs);
    }

    // Delete all log.csv files in the directory hierarchy
    void clear_logs(const char *path)
    {
        clear_file(path,"log.csv","clearlogs");
    }

    // Deletes a named filetype in the directory hierarchy
    void clear_file(const char *path,const char *filename,const char *invoke_name)
    {
        FILE *fp;
        char buf[PATH_MAX];
        int n_dirs;
        int i;

        n_dirs = directory_crawl(path);
        for (i = 0; i < n_dirs; i++) {
            // Search for a goldstars file in this directory
            strncpy(buf,directory[i],PATH_MAX);
            strncat(buf,filename,PATH_MAX-strlen(buf)-1);
            if (strlen(buf) >= PATH_MAX-1) {
                fprintf(stderr,"%s: pathname %s truncated, exiting.\n",invoke_name,buf);
                exit(EXIT_FAILURE);
            }
            fp = fopen(buf,"r");
            if (fp != NULL) {
                // We found such a file, delete it
                fclose(fp);
                if (remove(buf) < 0)
                    error(EXIT_FAILURE,errno,"%s: file:\n  %s",invoke_name,buf);
            }
        }

        free_directories(n_dirs);
    }


    // Extracts a single variable from a configuration file and puts the
    // string in the variable "value". Returns 1 on success and 0 on
    // failure.
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
                // Skip over the "=" sign
                tmpvalue++;
                // Skip whitespace
                while (isspace(*tmpvalue))
                    tmpvalue++;
                // Eliminate any whitespace at end
                param_begin = tmpvalue;
                tmpvalue = param_begin + strlen(param_begin) - 1;
                while (tmpvalue > param_begin && isspace(*tmpvalue)) {
                    *tmpvalue = '\0';
                    tmpvalue--;
                }
                // Abort if empty
                if (strlen(param_begin) == 0)
                    continue;
                // Successful, copy the result
                *value = strdup(param_begin);
                return 1;
            }
        }
        return 0;
    }

    // Recursively generates a list of all subdirectories listed in
    // user_menu_entries starting from the given path.  It populates the
    // global variables "directory" and "directory_level", and returns the
    // total number found.  Note this function allocated memory with
    // malloc, so after you're done using these directories you should
    // call free_directories.
    //
    // This function checks to make sure that each directory exists, and
    // exits if not, so you can be sure that all listed directories exist.
    //
    // Note this puts the top level directory (assigned in path) as the
    // first entry (which "main" verifies to be valid), so this function
    // is guaranteed to return at least one directory.
    int directory_crawl(const char *path)
    {
        int current_length;
        int previous_length;
        int current_level;
        FILE *fp;
        char buf[PATH_MAX];
        char fullpath[PATH_MAX];
        int isdone;
        int i;
        char *line_begin;
        DIR *dir;

        current_length = 1;
        directory[0] = (char*) malloc((strlen(path)+2)*sizeof(char));
        if (directory[0] == NULL) {
            fprintf(stderr,"Memory allocation error in directory_crawl.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(directory[0],path);
        // Append '/' if necessary
        if (directory[0][strlen(path)-1] != '/')
            strcat(directory[0],"/");
        current_level = 1;
        directory_level[0] = current_level;

        isdone = 0;
        while (!isdone) {
            previous_length = current_length;
            isdone = 1;  // We'll be finished if we don't find any new user_menu_entries files
            for (i = 0; i < previous_length; i++) {
                // Just parse directories of the most recently-added level
                // (we've already done the work for previous levels)
                if (directory_level[i] == current_level) {
                    // Read the user_menu_entries file, if it exists
                    // Note that previous items already have "/" appended, no need
                    // to worry about that here.
                    strncpy(fullpath,directory[i],PATH_MAX);
                    strncat(fullpath,USER_MENU_ENTRIES_FILENAME,PATH_MAX-strlen(fullpath)-1);
                    fp = fopen(fullpath,"r");
                    if (fp != NULL) {
                        // We found the user_menu_entries file, read it and add directories
                        while (fgets (buf, PATH_MAX, fp)) {
                            if (current_length >= MAX_USERS) {
                                fprintf(stderr,"Error: maximum number of users exceeded.");
                                exit(EXIT_FAILURE);
                            }
                            // Skip over leading & trailing white space, and
                            // especially blank lines
                            line_begin = eatwhite(buf);
                            if (strlen(line_begin) == 0)
                                continue;

                            directory[current_length] = (char *) malloc((strlen(directory[i])+strlen(line_begin)+2)*sizeof(char));
                            if (directory[current_length] == NULL) {
                                fprintf(stderr,"Memory allocation error in directory_crawl.\n");
                                exit(EXIT_FAILURE);
                            }
                            // Append each new directory to the list
                            strcpy(directory[current_length],directory[i]);
                            strcat(directory[current_length],line_begin);
                            strcat(directory[current_length],"/");
                            directory_level[current_length] = current_level+1;
                            // Check to make sure it's valid
                            dir = opendir(directory[current_length]);
                            if (dir == NULL)
                                error(EXIT_FAILURE,errno,"directory:\n %s",directory[current_length]);
                            closedir(dir);
                            current_length++;
                        }
                        isdone = 0;  // We know we need to check the subdirectories
                        fclose(fp);
                    }  // end of: if (fp != NULL)
                } // end of: if (directory_level[i] == current_level)
            } // end of: loop over previous directories
            current_level++;  // We're all done parsing this level, move on
        } // end of: while (!isdone)

        return current_length;
    }

    void free_directories(int n)
    {
        int i;

        for (i = 0; i < n; i++) {
            free(directory[i]);
            directory[i] = NULL;
        }
    }

    // Modify a string to eliminate leading and trailing
    // whitespace. Returns a pointer to the first non-space character.
    // Note the input string is modified in-place!
    char* eatwhite(char *buf)
    {
        char *line_begin, *line_end;

        // Eliminate leading whitespace
        line_begin = buf;
        while (isspace(*line_begin))
            line_begin++;
        // Eliminate trailing whitespace, especially the \n at the end of the line
        line_end = line_begin+strlen(line_begin)-1;
        while (line_end >= line_begin && isspace(*line_end)) {
            *line_end = '\0';
            line_end--;
        }

        return line_begin;
    }
