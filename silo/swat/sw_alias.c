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
 *
 *  Alias Package Function Summary
 *
 *
 *               alias_init()                   {Initialize package}
 *               alias_purge()                  {Purge all definitions}
 *      status = alais_set  (alias, replace)    {Set alias def'n}
 *      status = alias_get  (alias, replace)    {Get alias def'n}
 *      status = alias_undo (alias)             {Remove alias def'n}
 *      status = alias_exists (str)             {Return OKAY if str is alias}
 *      status = alias_show (alias)             {Print value of alias}
 *
 *
 *======================================================================*/

#define MAXALIAS   50

struct {
    char          *alias;
    char          *replace;
} aliases[MAXALIAS];

static int     nalias = 0;

/*----------------------------------------------------------------------
 *  Routine                                                    alias_init
 *
 *  Purpose
 *
 *      Initialize the alias package.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
void
alias_init(void)
{

    static int     init_done = 0;

    if (init_done)
        return;

    /* Initialize the static storage */
    alias_purge();

    init_done = 1;
}

/*----------------------------------------------------------------------
 *  Routine                                                  alias_purge
 *
 *  Purpose
 *
 *      Remove all definitions from the alias package.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
void
alias_purge(void)
{
    int            i;

    for (i = 0; i < MAXALIAS; i++) {

        aliases[i].alias = NULL;
        aliases[i].replace = NULL;
    }

    nalias = 0;
}

/*----------------------------------------------------------------------
 *  Routine                                                    alias_set
 *
 *  Purpose
 *
 *      Assign a value to the given alias.
 *
 *  Notes
 *
 *      Any existing alias by the given name is overwritten.
 *
 *  Assumptions
 *
 *  Modifications:
 *
 *    Lisa J. Roberts, Tue Nov 23 11:42:16 PST 1999
 *    Changed SAVESTRING to safe_strdup. 
 *
 *---------------------------------------------------------------------*/
int
alias_set(char *alias, char *replace)
{
    int            i;

    /* Validate parameters */
    if (alias == NULL || replace == NULL)
        return (OOPS);

    /* See if given alias has already been used */
    i = al_index(alias);

    if (i != OOPS) {

        FREE(aliases[i].replace);
        aliases[i].replace = safe_strdup(replace);
        return (OKAY);
    }

    /* Alias is new; find home for it and add it. */
    for (i = 0; i < MAXALIAS; i++) {

        if (aliases[i].alias == NULL) {
            aliases[nalias].alias = safe_strdup(alias);
            aliases[nalias].replace = safe_strdup(replace);
            nalias++;
            return (OKAY);
        }
    }

    /* No slots available for new alias. */
    printf("Insufficient space for new alias.\n");

    return (OOPS);
}

/*----------------------------------------------------------------------
 *  Routine                                                    alias_get
 *
 *  Purpose
 *
 *      Return the value assigned to the given alias.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
alias_get(char *alias, char *replace)
{
    int            i;

    /* Validate parameters */
    if (alias == NULL || replace == NULL)
        return (OOPS);

    replace[0] = '\0';

    /* See if given alias has a value */
    i = al_index(alias);

    if (i == OOPS)
        return (OOPS);

    strcpy(replace, aliases[i].replace);

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                   alias_undo
 *
 *  Purpose
 *
 *      Remove the entry for the given alias.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
alias_undo(char *alias)
{
    int            i;

    /* Validate parameters */
    if (alias == NULL)
        return (OOPS);

    /* See if given alias has a value */
    i = al_index(alias);

    if (i == OOPS)
        return (OOPS);

    /* Release storage for aliases and NULL out values */
    FREE(aliases[i].alias);
    FREE(aliases[i].replace);
    aliases[i].alias = NULL;
    aliases[i].replace = NULL;

    return (OKAY);
}

/*----------------------------------------------------------------------
 *  Routine                                                 alias_exists
 *
 *  Purpose
 *
 *      Return TRUE if an alias exists for the given string.
 *
 *  Notes
 *
 *
 *  Assumptions
 *
 *---------------------------------------------------------------------*/
int
alias_exists(char *str)
{
    int            i;

    /* Validate parameters */
    if (str == NULL)
        return (OOPS);

    /* See if given string has a value */
    i = al_index(str);

    if (i == OOPS)
        return (FALSE);
    else
        return (TRUE);
}

/*----------------------------------------------------------------------
 *  Routine                                                   alias_show
 *
 *  Purpose
 *
 *      Print the value assigned to the given alias, or the values of
 *      all aliases if none is supplied.
 *
 *---------------------------------------------------------------------*/
int
alias_show(char *alias)
{
    int            i;

    /* Handle case where ALL aliases were requested */
    if (alias == NULL) {

        for (i = 0; i < MAXALIAS; i++) {
            if (aliases[i].alias != NULL)
                printf("%s\t%s\n", aliases[i].alias, aliases[i].replace);
        }

        return (OKAY);
    }

    /* See if given alias has a value */
    i = al_index(alias);

    if (i == OOPS)
        return (OOPS);
    else {
        printf("%s\n", aliases[i].replace);
        return (OKAY);
    }
}

/*----------------------------------------------------------------------
 *  Routine                                                     al_index
 *
 *  Purpose
 *
 *      Return the index of the given alias, else return "OOPS" if
 *      not found.
 *
 *  Notes
 *
 *      This routine is for internal use only.
 *
 *---------------------------------------------------------------------*/
int
al_index(char *alias)
{
    int            i;

    /* Validate parameters */
    if (alias == NULL || nalias <= 0)
        return (OOPS);

    /* Search for the given alias. */
    for (i = 0; i < MAXALIAS; i++) {
        if ((aliases[i].alias != NULL) &&
            (STR_EQUAL(alias, aliases[i].alias)))
            return (i);
    }

    return (OOPS);
}
