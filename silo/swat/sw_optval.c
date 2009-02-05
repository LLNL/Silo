/*

                           Copyright 1991 - 1999
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

#include "swat.h"
#include <string.h>

/*======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================

  Module Name                                                  optlist.c

  Purpose

        This module contains application-level routines for manipulating
        the option list (OptList) data structure.

  Programmer

        Jeffery Long, NSSD/B

  Description


  Contents

        (OptList *) MakeOptlist ((int) length);
        (int)       SetOption ((Optlist *) optlist, (char *) option,
                               (byte *) value);
        (int)       DelOption ((Optlist *) optlist, (char *) option);
        (int)       FreeOptlist ((Optlist *) optlist);

  ======================================================================
  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
  ======================================================================*/

/*----------------------------------------------------------------------
 *  Routine                                                  MakeOptlist
 *
 *  Purpose
 *
 *      Create an optlist list of the requested length and return to
 *      caller.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *--------------------------------------------------------------------*/
Optlist *
MakeOptlist(int len)
{
    Optlist       *ol;

    /* Cursory error checking */
    if (len < 1 || len > 10000)
        return (NULL);

    /* Make an optlist of the requested length */
    ol = ALLOC(Optlist);

    ol->options = ALLOC_N(char *, len);
    ol->values = ALLOC_N(byte *, len);
    ol->noptions = 0;
    ol->list_len = len;

    return (ol);
}

/*----------------------------------------------------------------------
 *  Routine                                                    SetOption
 *
 *  Purpose
 *
 *      Given an option list, set the value for an option.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *      If the requested option already exists, the associated value
 *      is merely replaced; otherwise a new entry is made.
 *
 *      IMPORTANT:  The value parameter is not copied by this function,
 *                  so you must be careful not to release it or change
 *                  its value. Consider using the memsave function.
 *
 *                  'value' MUST BE ALLOCATED WITH MALLOC!!!!!
 *
 *  Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 11:43:58 PST 1999
 *    Changed STR_SAVE to safe_strdup.
 *
 *--------------------------------------------------------------------*/
int
SetOption(Optlist *ol, char *option, byte *value)
{
    int            i;
    int            found = FALSE;

    /* Do a little error checking */
    if (ol == NULL || ol->list_len < 1 || ol->noptions < 1)
        return (OOPS);

     /*-------------------------------------------------------
      *  Check for this option before adding to list. Just
      *  replace existing value if found.
      *-------------------------------------------------------*/
    for (i = 0; i < ol->list_len; i++) {
        if (STR_EQUAL(ol->options[i], option)) {
            FREE(ol->values);
            ol->values[i] = value;
            found = TRUE;
            break;
        }
    }

     /*-------------------------------------------------------
      *  If not found, find first available entry.
      *-------------------------------------------------------*/
    if (!found) {
        for (i = 0; i < ol->list_len; i++) {
            if (ol->options[i] == NULL) {
                ol->options[i] = safe_strdup(option);
                ol->values[i] = value;
                found = TRUE;
                ol->noptions++;
                break;
            }
        }
    }

    return (found ? OKAY : OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                                    DelOption
 *
 *  Purpose
 *
 *      Given an option list, delete an option and associated value.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *      IMPORTANT:  The value parameter is FREE'd by this function,
 *                  so you must be careful to only send values to
 *                  SetOption which were allocated with malloc.
 *
 *--------------------------------------------------------------------*/
int
DelOption(Optlist *ol, char *option)
{

    int            i;
    int            found = FALSE;

    /* Do a little error checking */
    if (ol == NULL || ol->list_len < 1 || ol->noptions < 1)
        return (OOPS);

     /*-------------------------------------------------------
      *  Find requested option and clear entry.
      *-------------------------------------------------------*/
    for (i = 0; i < ol->list_len; i++) {

        if (STR_EQUAL(ol->options[i], option)) {

            FREE(ol->values[i]);
            FREE(ol->options[i]);
            ol->options[i] = NULL;
            ol->noptions--;
            found = TRUE;
            break;
        }
    }

    return (found ? OKAY : OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                                  FreeOptlist
 *
 *  Purpose
 *
 *      Free an option list and all associated storage.
 *
 *  Programmer
 *
 *      Jeffery W. Long, NSSD-B
 *
 *  Notes
 *
 *      IMPORTANT:  The value parameter is FREE'd by this function,
 *                  so you must be careful to only send values to
 *                  SetOption which were allocated with malloc.
 *
 *--------------------------------------------------------------------*/
int
FreeOptlist(Optlist *ol)
{
    int            i;

    if (ol == NULL)
        return (OOPS);

    for (i = 0; i < ol->list_len; i++) {
        FREE(ol->options[i]);
        FREE(ol->values[i]);
    }

    FREE(ol);

    return (OKAY);
}
