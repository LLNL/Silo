#include <stdio.h>
#include <swat.h>
#include <string.h>
#include <stdlib.h>

#define ARG_TYPE_NONE       0
#define ARG_TYPE_STRING     1
#define ARG_TYPE_INT        2
#define ARG_TYPE_FLOAT      3
#define ARG_TYPE_DOUBLE     4
#define ARG_TYPE_CHAR       5
#define ARG_TYPE_UNKNOWN    6

/* File-wide modifications:
 *
 * Sean Ahern, Thu Dec  7 10:40:01 PST 2000
 * Renamed getarg and getarg_noremove to sw_getarg and sw_getarg_noremove
 * so they would stop conflicting with the Fortran getarg call.
 */

static void shift_em(int*,char***,int);
static int doit(void *arg, int *argc, char ***argv, char *optstring,
                int remove);

int
sw_getarg(void *arg, int *argc, char ***argv, char *optstring)
{
    return doit(arg,argc,argv,optstring,1);
}

int
sw_getarg_noremove(void *arg, int *argc, char ***argv, char *optstring)
{
    return doit(arg,argc,argv,optstring,0);
}


/* Function: doit
 *
 * Author: Sean Ahern (ahern@llnl.gov)
 * Date:   Fri Oct  7 12:41:54 PDT 1994
 *
 * Purpose:     Gets the next argument from the argument list that matches the
 *              given optstring.  It modifies the argc/argv argument list to 
 *              remove the argument after it has been parsed.
 *
 * The format of the optstring is:
 *
 *      argument:type
 *
 * where argument is the string to match in the argument list, and type is
 * one of the type identifiers given below:
 *
 *      Type                Identifier
 *      ------------------------------
 *      string (char*)      s
 *      int                 i
 *      float               f
 *      double              d
 *      char                c
 *      none                <none>
 *
 * For example, to match a "-max" option with a float parameter, the
 * optstring is "max:f".  This would appear as "-max 5.4" in the argv
 * array.  To match a "-foo" option with no paramter, the optstring is
 * "foo" with no type specified.
 *
 * The value of the option, if requested, it placed in the arg parameter,
 * for which space must have been allocated in the case of all types except
 * string.  String types will be allocated using malloc and returned.  For
 * options requesting no type, this parameter is ignored.
 *
 * Returns: 1 on success
 *          0 on argument not found in array
 *         -1 on bad or missing optlist
 *         -2 on parameter not found for argument
 *         -3 on parameter is invalid
 *
 * Notes:  Duplicate options are not checked.  The first is found, removed from
 * argc/argv, and the rest are ignored until sw_getarg is called again.
 *
 * Modifications:
 *     Brad Whitlock, Tue Apr 4 14:13:26 PST 2000
 *     Fixed a string under-allocation problem.
 *
 *     Hank Childs, Mon Oct  9 14:20:10 PDT 2000
 *     Added static keyword to prevent warning from not matching prototype.
 */
static int
doit(void *arg, int *argc, char ***argv, char *optstring, int remove)
{
    int             type, l;
    char           *option = NULL, *p = NULL;
    register int    i;
    char           *os = safe_strdup(optstring);
    char           *format = NULL;

    l = strlen(os);
    if ((!os) || (l == 0))
    {
        fprintf(stderr,"sw_getarg: optstring is NULL or invalid.\n");
        return -1;
    }

    option = strtok(os, ":");
    p = strtok(NULL, ":");

    if (p == NULL)
        type = ARG_TYPE_NONE;
    else
        switch (*p)
        {
        case 's':
            type = ARG_TYPE_STRING;
            format = safe_strdup("%s");
            break;
        case 'i':
            type = ARG_TYPE_INT;
            format = safe_strdup("%d");
            break;
        case 'f':
            type = ARG_TYPE_FLOAT;
            format = safe_strdup("%f");
            break;
        case 'd':
            type = ARG_TYPE_DOUBLE;
            format = safe_strdup("%lf");
            break;
        case 'c':
            type = ARG_TYPE_CHAR;
            format = safe_strdup("%c");
            break;
        default:
            type = ARG_TYPE_UNKNOWN;
            fprintf(stderr, "sw_getarg: Unknown option type '%c'.\n", *p);
            FREE(os);
            return -1;
        }

    for (i = 1; i < *argc; i++)
    {
        char *current_flag = (char *)((*argv)[i]);

        if ((current_flag[0] == '-') && (!strcmp(current_flag + 1, option)))
        {
            char *current_option = (char *)((*argv)[i + 1]);

            /* option found */
            if (type != ARG_TYPE_NONE)
            {
                char *straddr = NULL;

                if (i == *argc - 1)
                {
                    fprintf(stderr, "Parameter not found for -%s.\n", option);
                    FREE(os);
                    return -2;
                }
                if (type == ARG_TYPE_STRING)
                    straddr = (char *)malloc(sizeof(char) * (1 + strlen(current_option)));

                /* The following line causes a compiler warning about void*
                 * that really can't be avoided.  */
                if ((sscanf(current_option, format,
                            (type == ARG_TYPE_STRING) ? straddr : (arg))) != 1)
                {
                    switch (type)
                    {
                    case ARG_TYPE_STRING:
                        FREE(straddr);
                        fprintf(stderr, "String expected for -%s.\n", option);
                        break;
                    case ARG_TYPE_INT:
                        fprintf(stderr, "Integer expected for -%s.\n", option);
                        break;
                    case ARG_TYPE_FLOAT:
                        fprintf(stderr, "Float expected for -%s.\n", option);
                        break;
                    case ARG_TYPE_DOUBLE:
                        fprintf(stderr, "Double expected for -%s.\n", option);
                        break;
                    case ARG_TYPE_CHAR:
                        fprintf(stderr, "Character expected for -%s.\n", option);
                        break;
                    }
                    FREE(os);
                    FREE(format);
                    return -3;
                }
                FREE(format);

                if (remove)
                {
                    /* Get parameter out of argument list */
                    shift_em(argc, argv, i + 1);
                }

                /* If the option was a string, it was read correctly by now.
                 * We should return it. */
                if(type == ARG_TYPE_STRING)
                    *((char **)arg) = straddr; 
            }
            if (remove)
            {
                /* Get option out of argument list */
                shift_em(argc, argv, i);
            }

            FREE(os);
            return 1;
        }
    }

    FREE(os);
    return 0;
}

/* Function: shift_em
 *
 * Purpose:     Shifts the items in the argc/argv array, essentially deleting 
 *              the element out of position "pos".
 *
 * Modifications:
 */
static void
shift_em(int *argc, char ***argv, int pos)
{
    int i;

    for(i=pos;i<*argc;i++)
    {
        (*argv)[i] = (*argv)[i+1];
    }
    (*argv)[*argc-1] = NULL;
    (*argc)--;
}
