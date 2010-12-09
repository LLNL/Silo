/*
Copyright (c) 1994 - 2010, Lawrence Livermore National Security, LLC.
LLNL-CODE-425250.
All rights reserved.

This file is part of Silo. For details, see silo.llnl.gov.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted
     below) in the documentation and/or other materials provided with
     the distribution.
   * Neither the name of the LLNS/LLNL nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE  IS PROVIDED BY  THE COPYRIGHT HOLDERS  AND CONTRIBUTORS
"AS  IS" AND  ANY EXPRESS  OR IMPLIED  WARRANTIES, INCLUDING,  BUT NOT
LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A  PARTICULAR  PURPOSE ARE  DISCLAIMED.  IN  NO  EVENT SHALL  LAWRENCE
LIVERMORE  NATIONAL SECURITY, LLC,  THE U.S.  DEPARTMENT OF  ENERGY OR
CONTRIBUTORS BE LIABLE FOR  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR  CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT  LIMITED TO,
PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS  OF USE,  DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER  IN CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING
NEGLIGENCE OR  OTHERWISE) ARISING IN  ANY WAY OUT  OF THE USE  OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This work was produced at Lawrence Livermore National Laboratory under
Contract No.  DE-AC52-07NA27344 with the DOE.

Neither the  United States Government nor  Lawrence Livermore National
Security, LLC nor any of  their employees, makes any warranty, express
or  implied,  or  assumes  any  liability or  responsibility  for  the
accuracy, completeness,  or usefulness of  any information, apparatus,
product, or  process disclosed, or  represents that its use  would not
infringe privately-owned rights.

Any reference herein to  any specific commercial products, process, or
services by trade name,  trademark, manufacturer or otherwise does not
necessarily  constitute or imply  its endorsement,  recommendation, or
favoring  by  the  United  States  Government  or  Lawrence  Livermore
National Security,  LLC. The views  and opinions of  authors expressed
herein do not necessarily state  or reflect those of the United States
Government or Lawrence Livermore National Security, LLC, and shall not
be used for advertising or product endorsement purposes.
*/
/*-------------------------------------------------------------------------
 *
 * Created:             switch.c
 *                      2000-05-31
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             Parse browser command-line switches.
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
#include <browser.h>

static switch_t *switch_latest;
static char *switch_synopsis(switch_t *sw, char *buffer);

/*---------------------------------------------------------------------------
 * Purpose:     Create a new empty switch list.
 *
 * Return:      Pointer to new switch list object.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
switches_t *
switch_new(void)
{
    return calloc(1, sizeof(switches_t));
}

/*---------------------------------------------------------------------------
 * Purpose:     Add a new switch to the switch list or modify an existing
 *              switch.
 *
 * Return:      The switch added or modified.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
switch_t *
switch_add(switches_t *switches, const char *short_name, const char *long_name,
           const char *arg_spec, switch_handler_t handler)
{
    switch_t    *found = switch_find(switches, short_name);

    if (!found) found = switch_find(switches, long_name);
    if (found) {
        if (found->short_name) free(found->short_name);
        if (found->long_name) free(found->long_name);
        if (found->arg_spec) free(found->arg_spec);
        if (found->doc_string) free(found->doc_string);
    } else {
        if (switches->nused+1>=switches->nalloc) {
            switches->nalloc = MAX(100, 2*switches->nalloc);
            switches->sw = realloc(switches->sw,
                                   switches->nalloc*sizeof(switch_t));
        }
        found = switches->sw + switches->nused++;
    }
    
    memset(found, 0, sizeof(switch_t));
    found->all = switches;
    found->short_name = safe_strdup(short_name);
    found->long_name = safe_strdup(long_name);
    found->arg_spec = safe_strdup(arg_spec);
    found->handler = handler;
    switch_latest = found;
    return found;
}

/*---------------------------------------------------------------------------
 * Purpose:     Registers a caller-defined pointer with the switch.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
switch_info(switch_t *sw, void *info)
{
    if (!sw) sw = switch_latest;
    sw->info = info;
}

/*---------------------------------------------------------------------------
 * Purpose:     Register a documentation string with the switch.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
switch_doc(switch_t *sw, const char *doc_string)
{
    char        *fulldoc = malloc(8192);

    /* Set switch info */
    if (!sw) sw = switch_latest;
    if (sw->doc_string) free(sw->doc_string);
    sw->doc_string = safe_strdup(doc_string);

    /* Build browser documentation string */
    switch_synopsis(sw, fulldoc);
    strcat(fulldoc, "\n");
    strcat(fulldoc, doc_string);

    /* Assign browser documentation string to symbols */
    if (sw->short_name) {
        obj_t symbol = obj_new(C_SYM, sw->short_name);
        obj_t docstr = obj_new(C_STR, fulldoc);
        sym_dbind(symbol, docstr);
        obj_dest(symbol);
    }
    if (sw->long_name) {
        obj_t symbol = obj_new(C_SYM, sw->long_name);
        obj_t docstr = obj_new(C_STR, fulldoc);
        sym_dbind(symbol, docstr);
        obj_dest(symbol);
    }
    free(fulldoc);
}

/*---------------------------------------------------------------------------
 * Purpose:     Find a switch in a switch list. The name should include the
 *              leading hyphens.
 *
 * Return:      The switch found, or NULL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
switch_t *
switch_find(switches_t *switches, const char *name)
{
    size_t         i;

    if (name && '-'==name[0] && '-'==name[1]) {
        for (i=0; i<switches->nused; i++) {
            if (switches->sw[i].long_name &&
                !strcmp(switches->sw[i].long_name, name)) {
                return switches->sw+i;
            }
        }
    } else if (name && '-'==name[0]) {
        for (i=0; i<switches->nused; i++) {
            if (switches->sw[i].short_name &&
                !strcmp(switches->sw[i].short_name, name)) {
                return switches->sw+i;
            }
        }
    } else {
        for (i=0; i<switches->nused; i++) {
            if (!switches->sw[i].short_name &&
                !switches->sw[i].long_name) {
                return switches->sw+i;
            }
        }
    }
    return NULL;
}

/*---------------------------------------------------------------------------
 * Purpose:     Render into a string a one-line description of a switch.
 *
 * Return:      The result buffer.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  6, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
static char *
switch_synopsis(switch_t *sw, char *buffer)
{
    int         type, required;
    char        name[32];
    
    switch_arg(sw, &type, sizeof name, name, &required, NULL);
    buffer[0] = '\0';
        
    /* Short */
    if (sw->short_name) {
        strcat(buffer, sw->short_name);
        if (type) {
            strcat(buffer, " ");
            if (!required) strcat(buffer, "[");
            strcat(buffer, name);
            if (!required) strcat(buffer, "]");
        }
    }

    /* Long */
    if (sw->long_name) {
        if (buffer[0]) strcat(buffer, ", ");
        strcat(buffer, sw->long_name);
        if (type) {
            if (!required) strcat(buffer, "[");
            strcat(buffer, "=");
            strcat(buffer, name);
            if (!required) strcat(buffer, "]");
        }
    }
    return buffer;
}

/*---------------------------------------------------------------------------
 * Purpose:     Prints a usage message to the standard error stream.  If
 *              SNAME is non-null then print only usage information for
 *              that particular switch, otherwise print full usage
 *              information.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
switch_usage(switches_t *switches, const char *arg0, const char *sname)
{
    size_t      i;
    char        synopsis[256];

    /* Base name of executable */
    const char  *base = strrchr(arg0, '/');
    base = base ? base+1 : arg0;

    if (!sname) {
        sprintf(synopsis, "usage: %s [SWITCHES] [--] [FILES]", base);
        out_line(OUT_STDERR, synopsis);
        out_line(OUT_STDERR, "  Where SWITCHES are:");
    }

    for (i=0; i<switches->nused; i++) {
        switch_t *sw = switches->sw+i;
        if (sname &&
            (!sw->short_name || strcmp(sw->short_name, sname)) &&
            (!sw->long_name || strcmp(sw->long_name, sname))) {
            continue;
        }
        out_line(OUT_STDERR, switch_synopsis(sw, synopsis));
        out_putw(OUT_STDERR, sw->doc_string);
        out_nl(OUT_STDERR);
    }
}

/*---------------------------------------------------------------------------
 * Purpose:     Return information about the argument of a switch. The
 *              TYPE is a single character (g=floating point, s=string,
 *              d=integer, u=unsigned integer, b=boolean). The NAME will
 *              be initialized with the first NAME_SIZE characters of the
 *              name of the argument (one is created if none specified).
 *              The NAME will be the empty string if no argument is
 *              allowed for the switch. REQUIRED will be set to zero if
 *              the argument is optional and non-zero otherwise. The DFLT
 *              will be set to point to the default argument value if the
 *              argument is optional.
 *
 * Programmer:  Robb Matzke
 *              Thursday, June  1, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
switch_arg(switch_t *sw, int *type, size_t name_size, char *name,
           int *required, const char **dflt)
{
    const char  *s = sw->arg_spec, *eq;
    char        local_type;
    size_t      len;

    /* Initialize return values */
    if (type) *type = '\0';
    if (name_size && name) name[0] = '\0';
    if (required) *required = 0;
    if (dflt) *dflt = NULL;

    /* Return immediately if no argument is allowed. */
    if (!s || !*s) return;

    /* The type letter is always first */
    if (type) *type = *s;
    local_type = *s++;

    /* The name is always preceeded by a colon. If the colon is missing
     * then generate some generic name */
    if (':'==*s) {
        s++;
        if ((eq=strchr(s, '='))) {
            len = eq-s;
        } else {
            len = strlen(s);
        }
        if (name_size && name) {
            strncpy(name, s, MIN(name_size, len));
            name[MIN(name_size-1,len)] = '\0';
        }
        s += len;
    } else if (name_size && name) {
        switch (local_type) {
        case 'g':
            strncpy(name, "NUMBER", name_size);
            break;
        case 'd':
        case 'u':
            strncpy(name, "INTEGER", name_size);
            break;
        case 's':
            strncpy(name, "STRING", name_size);
            break;
        case 'b':
            strncpy(name, "BOOLEAN", name_size);
            break;
        default:
            name[0] = local_type;
            break;
        }
        name[name_size-1] = '\0';
    }

    /* If an equal sign is next then the argument is optional and might
     * have some default value. */
    if ('='==*s) {
        s++;
        if (dflt) *dflt = s;
    } else {
        *required = true;
    }
}

/*---------------------------------------------------------------------------
 * Purpose:     Default error printing function
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
static void
switch_error(const char *fmt, ...)
{
    va_list     ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

/*---------------------------------------------------------------------------
 * Purpose:     This function gets called to parse the switch value based
 *              on the switch's arg spec. If RELAX is non-zero then an
 *              error is treated as if the argument is not present.
 *
 * Programmer:  Robb Matzke
 *              Thursday, June  1, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
int
switch_parse_arg(switch_t *sw, const char *argv, const char *value,
                 int relax, void(*error)(const char*, ...))
{
    int         i, type, required, d, retval=0;
    char        *rest;
    const char  *tmp;
    double      g;
    const char  *sw_name = sw->long_name?sw->long_name:sw->short_name;
    const char  *dflt=NULL;
    
    switch_arg(sw, &type, 0, NULL, &required, &dflt);
    if (required && !value) {
        (error)("switch `%s' requires an argument", sw_name);
        return -1;
    }

    switch (type) {
    case 0:
         /* no argument possible */
        return 0;
        
    case 'g':
        /* floating-point argument */
        for (i=0; i<2; i++) {
            if (NULL==(tmp=i?dflt:value) || !*tmp) continue;
            g = strtod(tmp, &rest);
            if (rest && *rest) {
                if (!relax) {
                    (error)("switch `%s' should have a floating-point "
                            "argument", sw_name);
                    return -1;
                }
            } else {
                sw->lexeme = tmp;
                sw->value.g = g;
                retval = i?0:1;
                break;
            }
        }
        break;
        
    case 'd':
        /* integer argument */
        for (i=0; i<2; i++) {
            if (NULL==(tmp=i?dflt:value) || !*tmp) continue;
            d = strtol(tmp, &rest, 0);
            if (rest && *rest) {
                if (!relax) {
                    (error)("switch `%s' should have an integer argument",
                            sw_name);
                    return -1;
                }
            } else {
                sw->lexeme = tmp;
                sw->value.d = d;
                retval = i?0:1;
                break;
            }
        }
        break;

    case 'u':
        /* unsigned integer argument */
        for (i=0; i<2; i++) {
            if (NULL==(tmp=i?dflt:value) || !*tmp) continue;
            d = strtol(tmp, &rest, 0);
            if ((rest && *rest) || d<0) {
                if (!relax) {
                    (error)("switch `%s' should have a non-negative integer "
                            "argument", sw_name);
                    return -1;
                }
            } else {
                sw->lexeme = tmp;
                sw->value.d = d;
                retval = i?0:1;
                break;
            }
        }
        break;

    case 's':
        /* string argument */
        for (i=0; i<2; i++) {
            if (NULL==(tmp=i?dflt:value)) continue; /*empty string okay*/
            sw->lexeme = tmp;
            sw->value.s = tmp;
            retval = i?0:1;
            break;
        }
        break;

    case 'b':
        /* boolean */
        for (i=0; i<2; i++) {
            if (NULL==(tmp=i?dflt:value)) continue; /*empty string okay*/
            if (!tmp[0] ||
                !strcmp(tmp, "f") ||
                !strcmp(tmp, "false") ||
                !strcmp(tmp, "n") ||
                !strcmp(tmp, "no")) {
                sw->lexeme = tmp;
                sw->value.d = 0;
                retval = i?0:1;
                break;
            } else if (!strcmp(tmp, "t") ||
                       !strcmp(tmp, "true") ||
                       !strcmp(tmp, "y") ||
                       !strcmp(tmp, "yes")) {
                sw->lexeme = tmp;
                sw->value.d = 1;
                retval = i?0:1;
                break;
            } else {
                d = strtol(tmp, &rest, 0);
                if (!rest || !*rest) {
                    sw->lexeme = tmp;
                    sw->value.d = d;
                    retval = i?0:1;
                    break;
                } else if (!relax) {
                    (error)("switch `%s' should have a Boolean argument",
                            sw_name);
                    return -1;
                }
            }
        }
        break;

    default:
        abort();
    }
    return retval;
}

/*---------------------------------------------------------------------------
 * Audience:    Public
 * Purpose:     Parse command-line switches
 *
 * Return:      Number of arguments parsed or negative on error.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, May 31, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
int
switch_parse(switches_t *switches, int argc, char *argv[],
             void(*error)(const char*, ...))
{
    int         i, status;
    switch_t    *dflt = switch_find(switches, NULL);

    /* Error handling */
    if (!error) error = switch_error;

    /* Parse switches */
    for (i=1; i<argc; i++) {
        switch_t        *found  = NULL;
        const char      *option = NULL;

        if (!strcmp(argv[i], "--")) {
            return i+1; /*last argument*/
            
        } else if ('-'==argv[i][0] && '-'==argv[i][1]) {
            /* Long arguments */
            const char  *eq = strchr(argv[i], '=');
            size_t      namelen = eq ? eq-argv[i] : strlen(argv[i]);
            char        name[1024];

            strncpy(name, argv[i], namelen);
            name[namelen] = '\0';
            option = eq ? eq+1 : NULL;
            found = switch_find(switches, name);
            if (!found) found = dflt;
            if (!found) {
                (error)("unknown switch `%s'", argv[i]);
                return -1;
            }
            status = switch_parse_arg(found, argv[i], option, false, error);
            if (status<0) return -1;
            if (found->handler) {
                (found->handler)(found, argv[i], option);
            }
            found->seen++;
            
        } else if ('-'==argv[i][0] && argv[i][1]) {
            /* Single-letter switches */
            const char  *s;

            for (s=argv[i]+1; *s; s++) {
                char tmp[3];
                sprintf(tmp, "-%c", *s);
                found = switch_find(switches, tmp);
                if (!found) found = dflt;
                if (!found) {
                    (error)("unknown switch `%s'", tmp);
                    return -1;
                }
                option = s[1] ? s+1 : (i+1<argc ? argv[i+1] : NULL);
                status = switch_parse_arg(found, tmp, option, true, error);
                if (status<0) return -1;
                if (found->handler) {
                    (found->handler)(found, tmp, option);
                }
                found->seen++;
                if (status) {
                    if (s[1]) while (s[1]) s++;
                    else i++;
                    break;
                }
            }
            
        } else {
            /* Default */
            found = dflt;
            if (!found) return i; /*not a switch*/
            status = switch_parse_arg(found, argv[i], NULL, false, error);
            if (status<0) return -1;
            if (found->handler) {
                (found->handler)(found, argv[i], NULL);
            }
            found->seen++;
        }
    }
    return argc;
}
