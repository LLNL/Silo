/*

                           Copyright 1991 - 2002
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/

#if defined(_WIN32)
#include <silo_win32_compatibility.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                   sw_etc.c

  Purpose

       This module contains miscellaneous low-level functions used
       either by the SILO system or by anyone.

  Programmer

       Jeffery Long, NSSD/B

  Contents

       (char **)   FileList (dir, patt, nfiles)
       (int)       copy_var (from, to, nbytes)
       (void *)    memsave (ptr, nbytes)
       (int)       myfree  (ptr)
       (int)       search (array, item, length)
       (int)       malloc_debug (level)
       (int)       SW_file_readable (filename)

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

#include <stdio.h>
#include <string.h>
#include "swat.h"
#include <sys/types.h>
#include <sys/stat.h>

#define TMP_FILE    "/tmp/listing"
#define MAXFILES    512

char           tmpbuf[512];
char           command[512];

/*----------------------------------------------------------------------
 *  Routine                                                     FileList
 *
 *  Purpose
 *
 *      Return the list of files found in the given directory which
 *      begin with the given pattern.
 *
 *  Usage
 *
 *      int   nfiles;
 *      char  **files;
 *
 *      files = FileList (current_dir, "xyz*", &nfiles);
 *
 *  Modified
 *     
 *      Lisa J. Nafziger, Fri Sep 27 14:27:38 PDT 1996
 *      Added "csh -f -c" to the "/bin/ls" command so that the
 *      C shell would be run instead of the Bourne shell.  This
 *      provides proper expansion of the ~ operator.
 *
 *      Brad Whitlock, Wed Feb 16 15:46:17 PST 2000
 *      Made the function return NULL is NOTUNIX is defined.
 *
 *----------------------------------------------------------------------*/
char **
FileList(char *dir, char *patt, int *nfiles)
{
#ifdef NOTUNIX
    return NULL;
#else
    char         **list;
    char         **master_list;
    int            i, n;
    FILE          *fp;

    /* Initializations. */
    n = 0;

    /*----------------------------------------------------------
     * Generate system command for listing requested file names.
     * Output will be sent to temporary file.  We use the C
     * shell so that tildes will be expanded correctly.
     *--------------------------------------------------------*/
    strcpy(command, "csh -f -c \'/bin/ls -d ");
    if (dir != NULL && dir[0] != '\0') {
        strcat(command, dir);
        strcat(command, "/");
    }
    strcat(command, patt);
    strcat(command, " > ");
    strcat(command, TMP_FILE);
    strcat(command, "\'");

    if ((system(command)) != 0)
        return (NULL);

    fp = fopen(TMP_FILE, "r");

    /* Allocate space for maximum number of files */
    master_list = (char **)malloc(sizeof(char *) * MAXFILES);

    /*----------------------------------------------------------
     * Loop through entire temporary file, allocating space for,
     * and storing each name separately.
     *--------------------------------------------------------*/
    while (fgets(tmpbuf, 512, fp) != NULL) {

        master_list[n] = (char *)malloc(strlen(tmpbuf) + 1);
        tmpbuf[strlen(tmpbuf) - 1] = '\0';  /* Remove trailing newline */

        strcpy(master_list[n], tmpbuf);

        n++;
    }
    fclose(fp);

    /*----------------------------------------------------------
     * Copy master list into a list for user.  Free master list.
     *---------------------------------------------------------*/
    list = (char **)malloc(sizeof(char *) * n);

    for (i = 0; i < n; i++)
        list[i] = master_list[i];

    free(master_list);

    *nfiles = n;
    return (list);
#endif
}

/*-----------------------------------------------------------------------
 *  MODULE                                                       copy_var
 *
 *  PURPOSE
 *
 *      Copy the source variable into the sink variable for 'nbytes'
 *      bytes.
 *
 *
 *  NOTES
 *
 *      This is useful for copying one structure into another without
 *      needing to know the exact contents.
 *
 *-------------------------------------------------------------------- */
void
copy_var(char *from, char *to, int nbytes)
{
    while (nbytes-- > 0)
        *to++ = *from++;
}

/*-----------------------------------------------------------------------
 *  Routine                                                      memsave
 *
 *  Purpose
 *
 *      Allocate space for the given array, and copy the given array
 *      into the new space.
 *
 *
 *-------------------------------------------------------------------- */
void *
memsave(char *ptr, int nbytes)
{
    char          *new;

    if (nbytes <= 0)
        return (NULL);

    new = (char *)malloc(nbytes);

    if (new == NULL)
        return (NULL);

    copy_var(ptr, new, nbytes);

    return (new);
}

/*--------------------------------------------------
 * MYFREE -- Free space; set pointer to NULL.
 *------------------------------------------------*/
void
myfree(int *ptr)
{
    if (ptr != NULL) {
        free(ptr);
        ptr = NULL;
    }
}

/*----------------------------------------------------------------------
 *  ROUTINE                                                       search
 *
 *  PURPOSE
 *
 *      Search an array for a particular item.  If found, return the
 *      item's index; otherwise, return -1.
 *
 *
 *  PROGRAMMER
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  SYNTAX & PARAMETERS
 *
 *      int search (void array, void item, int length);
 *
 *      search    {Out}   {0-origin index if found, else -1}
 *      array     {In}    {Array of generic type}
 *      item      {In}    {Item to search 'array' for}
 *      length    {In}    {Length of 'array'}
 *
 *  NOTES
 *
 *      The parms are declared as type 'int' so only integer arrays
 *      can be searched.
 *
 *--------------------------------------------------------------------*/

int
search(int *array, int item, int length)
{
    int            i, index;

    index = -1;

    for (i = 0; i < length; i++) {
        if (array[i] == item) {
            index = i;
            break;
        }
    }

    return (index);
}

/*----------------------------------------------------------------------
 *  Routine                                             SW_file_readable
 *
 *  Purpose
 *
 *      Determine if the given file has the correct read permissions set.
 *
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *      The permissions are checked as they are in Unix. That is, if
 *      the file is owned by the person running the program, then the
 *      read permissions for the owner MUST be on. If the file belongs
 *      to the same group as the person running the program, the read
 *      permissions for the group MUST be on. Otherwise, the read
 *      permissions for the world MUST be on. If any of the above are
 *      false, then FALSE is returned.
 *
 *--------------------------------------------------------------------*/
int
SW_file_readable(char *filename)
{
    struct stat    buf;

    if (-1 == stat(filename, &buf))
        return (FALSE);

    if (0 == access(filename, R_OK))
        return (TRUE);
    else
        return (FALSE);
}

/*----------------------------------------------------------------------
 *  Routine                                               SW_file_exists
 *
 *  Purpose
 *
 *      Determine if the given file exists.
 *
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *
 *--------------------------------------------------------------------*/
int
SW_file_exists(char *filename)
{

    struct stat    buf;

    if (-1 == stat(filename, &buf))
        return (FALSE);

    return (TRUE);
}
