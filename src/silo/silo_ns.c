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
Contract  No.   DE-AC52-07NA27344 with  the  DOE.  Neither the  United
States Government  nor Lawrence  Livermore National Security,  LLC nor
any of  their employees,  makes any warranty,  express or  implied, or
assumes   any   liability   or   responsibility  for   the   accuracy,
completeness, or usefulness of any information, apparatus, product, or
process  disclosed, or  represents  that its  use  would not  infringe
privately-owned   rights.  Any  reference   herein  to   any  specific
commercial products,  process, or  services by trade  name, trademark,
manufacturer or otherwise does not necessarily constitute or imply its
endorsement,  recommendation,   or  favoring  by   the  United  States
Government or Lawrence Livermore National Security, LLC. The views and
opinions  of authors  expressed  herein do  not  necessarily state  or
reflect those  of the United  States Government or  Lawrence Livermore
National  Security, LLC,  and shall  not  be used  for advertising  or
product endorsement purposes.
*/
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <silo_private.h>

typedef struct _DBexprnode {
    char type;
    int val;
    char sval[128];
    struct _DBexprnode *left;
    struct _DBexprnode *right;
} DBexprnode;

static void
FreeTree(DBexprnode *tree)
{
    if (!tree)
        return;
    FreeTree(tree->left);
    FreeTree(tree->right);
    free(tree);
}

static DBexprnode *
UpdateTree(DBexprnode *tree, const char t, int v, char *s)
{
    DBexprnode *retval = 0;
    DBexprnode *newnode = (DBexprnode *) calloc(1,sizeof(DBexprnode));
    newnode->type = t;
    if (t == 'c')
        newnode->val = v;
    else 
    {
        if (s)
            strncpy(newnode->sval, s, sizeof(newnode->sval)-1);
        else
            strcpy(newnode->sval, "(null)");
    }

    newnode->left = 0;
    newnode->right = 0;

    if (tree == 0)
    {
        retval = newnode;
    }
    else if (tree->left == 0 && tree->right == 0)
    {
        /* t better be an operator */
        newnode->left = tree;
        retval = newnode;
    }
    else if (tree->left != 0 && tree->right == 0)
    {
        /* t better be a constant */
        tree->right = newnode;
        retval = tree;
    }
    else if (tree->left == 0 && tree->right != 0)
    {
        /* should never happen */
        ;
    }
    else
    {
        /* t better be an operator */
        newnode->left = tree;
        retval = newnode;
    }

    return retval;
}

static DBexprnode *
BuildExprTree(const char **porig)
{
    DBexprnode *tree = 0;
    const char *p = *porig;

    while (*p != '\0')
    {
        switch (*p)
        {
            case ' ':
            {
                break; /* ignore spaces */
            }

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            {
                char tokbuf[256];
                char *tp = tokbuf;
                long int val;
                while ('0' <= *p && *p <= '9')
                    *tp++ = *p++;
                p--;
                *tp = '\0';
                errno = 0;
                val = strtol(tokbuf, 0, 0);
                if (errno == 0 && val != LONG_MIN && val != LONG_MAX)
                    tree = UpdateTree(tree, 'c', (int) val, 0);
                break;
            }

            case '\'': /* beginning of string */
            {
                char tokbuf[129];
                char *tp = tokbuf;
                p++;
                while (*p != '\'')
                    *tp++ = *p++;
                *tp = '\0';
                errno = 0;
                tree = UpdateTree(tree, 's', 0, tokbuf);
                break;
            }

            case '$': /* array ref */
            case '#':
            {
                char typec = *p;
                char tokbuf[129];
                char *tp = tokbuf;
                DBexprnode *subtree;
                p++;
                while (*p != '[')
                    *tp++ = *p++;
                p--;
                *tp = '\0';
                errno = 0;
                tree = UpdateTree(tree, typec, 0, tokbuf);
                p++;
                subtree = BuildExprTree(&p);
                if (tree->left == 0)
                    tree->left = subtree;
                else if (tree->right == 0)
                    tree->right = subtree;
                break;
            }

            case 'n':
            case '+': case '-': case '*': case '/': case '%':
            case '|': case '&': case '^':
            {
                tree = UpdateTree(tree, *p, 0, 0);
                break;
            }

            case '(':
            {
                DBexprnode *subtree;
                p++;
                subtree = BuildExprTree(&p);
                if (tree == 0)
                    tree = subtree;
                else if (tree->left == 0)
                    tree->left = subtree;
                else if (tree->right == 0)
                    tree->right = subtree;
                break;
            }

            case ']': /* terminating array ref */
            case ')': /* terminating subtree */
            {
                *porig = p;
                return tree;
            }

            case '?':
            {
                DBexprnode *newtreeq = 0;
                DBexprnode *newnodec = 0;
                DBexprnode *subtreel, *subtreer;
                newtreeq = UpdateTree(newtreeq, *p, 0, 0);
                newtreeq->left = tree;
                p++;
                subtreel = BuildExprTree(&p);
                p++;
                subtreer = BuildExprTree(&p);
                newnodec = UpdateTree(newnodec, ':', 0, 0);
                newnodec->left = subtreel;
                newnodec->right = subtreer;
                newtreeq->right = newnodec;
                tree = newtreeq;
                break;
            }

            case ':': /* terminating if-then-else (?::) */
            {
                *porig = p;
                return tree;
            }


        }
        p++;
    }
    *porig = p;
    return tree;
}

/* very simple circular cache for a handful of embedded strings */
static int SaveString(DBnamescheme *ns, const char *sval)
{
    int modn = ns->nembed++ % DB_MAX_EXPSTRS;
    FREE(ns->embedstrs[modn]);
    ns->embedstrs[modn] = STRDUP(sval);
    return modn;
}

static int
EvalExprTree(DBnamescheme *ns, DBexprnode *tree, int n)
{
    if (tree == 0)
        return 0;
    else if ((tree->type == '$' || tree->type == '#') && tree->left != 0)
    {
        int i, q = EvalExprTree(ns, tree->left, n);
        for (i = 0; i < ns->narrefs; i++)
        {
            if (strcmp(tree->sval, ns->arrnames[i]) == 0)
            {
                if (tree->type == '$')
                    return SaveString(ns,  ((char**)ns->arrvals[i])[q]);
                else
                    return ns->arrvals[i][q];
            }
        }
    }
    else if (tree->left == 0 && tree->right == 0)
    {
        if (tree->type == 'c')
            return tree->val;
        else if (tree->type == 'n')
            return n;
        else if (tree->type == 's')
            return SaveString(ns, tree->sval);
    }
    else if (tree->left != 0 && tree->right != 0)
    {
        int vc = 0, vl, vr;
        if (tree->type == '?')
        {
            vc = EvalExprTree(ns, tree->left, n);
            tree = tree->right;
        }
        vl = EvalExprTree(ns, tree->left, n);
        vr = EvalExprTree(ns, tree->right, n);
        switch (tree->type)
        {
            case '+': return vl + vr;
            case '-': return vl - vr;
            case '*': return vl * vr;
            case '/': return vl / vr;
            case '%': return vl % vr;
            case '|': return vl | vr;
            case '&': return vl & vr;
            case '^': return vl ^ vr;
            case ':': return vc ? vl : vr; 
        }
    }
    return 0;
}

PUBLIC DBnamescheme *
DBMakeNamescheme(const char *fmt, ...)
{
    va_list ap;
    int i, j, k, n, pass, ncspecs, done;
    DBnamescheme *rv = 0;

    /* We have nothing to do for a null or empty format string */
    if (fmt == 0 || *fmt == '\0')
        return 0;

    /* Start by allocating an empty name scheme */
    rv = DBAllocNamescheme();
    
    /* set the delimeter character */
    rv->delim = fmt[0];

    /* compute length up to max of 4096 of initial segment of fmt representing
       the printf-style format string. */
    n = 1;
    while (n < 4096 && fmt[n] != '\0' && fmt[n] != rv->delim)
        n++;
    if (n == 4096) /* we pick arb. upper bound in length of 4096 */
    {
        DBFreeNamescheme(rv);
        return 0;
    }

    /* grab just the part of fmt that is the printf-style format string */
    rv->fmt = STRNDUP(&fmt[1],n-1);
    rv->fmtlen = n-1;

    /* In 2 passes, count conversion specs. and then setup pointers to each */ 
    for (pass = 0; pass < 2; pass++)
    {
        if (pass == 1)
        {
            rv->fmtptrs = (const char **) calloc(rv->ncspecs+1, sizeof(char*));
            rv->ncspecs = 0;
        }
        for (i = 0; i < rv->fmtlen-1; i++)
        {
            if (rv->fmt[i] == '%' && 
                rv->fmt[i+1] != '%')
            {
                if (pass == 1)
                    rv->fmtptrs[rv->ncspecs] = &(rv->fmt[i]);
                rv->ncspecs++;
            }
        }
    }
    rv->fmtptrs[rv->ncspecs] = &(rv->fmt[n+1]);

    /* If there are no conversion specs., we have nothing to do */
    if (rv->ncspecs == 0)
        return rv;

    /* Make a pass through rest of fmt string to count array refs in the
       expression substrings. */
    i = n+1;
    while (i < 4096 && fmt[i] != '\0')
    {
        if (fmt[i] == '$' || fmt[i] == '#')
            rv->narrefs++;
        i++;
    }
    if (i == 4096)
    {
        DBFreeNamescheme(rv);
        return 0;
    }

    /* allocate various arrays needed by the naming scheme */
    rv->exprstrs = (char **) calloc(rv->ncspecs, sizeof(char*));
    if (rv->narrefs > 0)
    {
        rv->arrnames = (char **) calloc(rv->narrefs, sizeof(char*));
        rv->arrvals  = (const int **) calloc(rv->narrefs, sizeof(int*));
    }

    /* Ok, now go through rest of fmt string a second time and grab each
       expression that goes with each conversion spec. Also, handle array refs */
    i = n+1;
    rv->narrefs = 0;
    ncspecs = 0;
    va_start(ap, fmt);
    done = 0;
    while (!done)
    {
        if (fmt[i] == '$' || fmt[i] == '#')
        {
            for (j = 1; fmt[i+j] != '['; j++)
                ;
            for (k = 0; k < rv->narrefs; k++)
            {
                if (strncmp(&fmt[i+1],rv->arrnames[k],j-1) == 0)
                    break;
            }
            if (k == rv->narrefs)
            {
                rv->arrnames[k] = STRNDUP(&fmt[i+1], j-1);
                rv->arrvals[k] = va_arg(ap, const int *);
                rv->narrefs++;
            }
        }
        else if (fmt[i] == rv->delim || fmt[i] == '\0')
        {
            rv->exprstrs[ncspecs] = STRNDUP(&fmt[n+1],i-(n+1));
            ncspecs++;
            if (fmt[i] == '\0' ||
                (fmt[i] == rv->delim && fmt[i] == '\0'))
                done = 1;
            n = i;
        }
        i++;
    }
    va_end(ap);

    return rv;
}

PUBLIC const char *
DBGetName(DBnamescheme *ns, int natnum)
{
    char *currentExpr, *tmpExpr;
    static char retval[1024];
    int i;

    retval[0] = '\0';
    strncat(retval, ns->fmt, ns->fmtptrs[0] - ns->fmt);
    for (i = 0; i < ns->ncspecs; i++)
    {
        char tmp[256];
        char tmpfmt[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        DBexprnode *exprtree;
        int theVal;

        currentExpr = STRDUP(ns->exprstrs[i]);
        tmpExpr = currentExpr;
        exprtree = BuildExprTree((const char **)&currentExpr);
        theVal = EvalExprTree(ns, exprtree, natnum);
        FreeTree(exprtree);
        strncpy(tmpfmt, ns->fmtptrs[i], ns->fmtptrs[i+1] - ns->fmtptrs[i]);
        if (strcmp(tmpfmt, "%s") == 0)
            sprintf(tmp, tmpfmt, ns->embedstrs[theVal]);
        else
            sprintf(tmp, tmpfmt, theVal);
        strcat(retval, tmp);
        FREE(tmpExpr);
    }
    return retval;
}
