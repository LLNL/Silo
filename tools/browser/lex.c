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
 * Created:             lex.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Lexical analysis functions.
 *
 * Modifications:
 *      Sean Ahern, Mon Oct 12 17:43:52 PDT 1998
 *      Removed references to AIO, since it's no longer supported.
 *      Converted tabs to spaces, removed trailing whitespace.
 *
 *      Thomas Treadway, Thu Jun  8 16:56:35 PDT 2006
 *      Modified readline definitions to support new configure macro.
 *
 *-------------------------------------------------------------------------
 */
#include "config.h"     /*MeshTV configuration record*/

#include <assert.h>
#include "browser.h"
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else /* !defined(HAVE_READLINE_H) */
extern char *readline ();
#  endif /* !defined(HAVE_READLINE_H) */
/***char *cmdline = NULL;***/ /* defined in main */
#else /* !defined(HAVE_READLINE_READLINE_H) */
  /* no readline */
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else /* !defined(HAVE_HISTORY_H) */
extern void add_history ();
extern int write_history ();
extern int read_history ();
#  endif /* defined(HAVE_READLINE_HISTORY_H) */
  /* no history */
#endif /* HAVE_READLINE_HISTORY */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#define LEX_PROMPT      "> "
#define LEX_PROMPT2     "...> "

/*
 * Non-posix functions
 */
lex_t *LEX_STDIN = NULL;

#ifndef _WIN32
extern FILE *fdopen(int, const char *);
#endif


/*-------------------------------------------------------------------------
 * Function:    lex_open
 *
 * Purpose:     Open a file for reading.
 *
 * Return:      Success:        Ptr to a lex_t input file.
 *
 *              Failure:        NULL, error printed.
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-06-07
 *              This function fails unless FNAME is a readable _file_.
 *
 *              Robb Matzke, 2000-07-10
 *              Sets LEX_STDIN if unset.
 *
 *      Thomas R. Treadway, Tue Jun 27 13:59:21 PDT 2006
 *      Added HAVE_STRERROR wrapper
 *-------------------------------------------------------------------------
 */
lex_t *
lex_open(const char *fname)
{
    lex_t       *f=NULL;
    struct stat sb;
    FILE        *stream;

    /* Check the file */
    if (stat(fname, &sb)<0) {
#ifdef HAVE_STRERROR
        out_errorn("lex_open: cannot open file `%s' (%s)",
                   fname, strerror(errno));
#else
        out_errorn("lex_open: cannot open file `%s' (errno=%d)",
                   fname, errno);
#endif
        return NULL;
    }
    if (!S_ISREG(sb.st_mode)) {
        out_errorn("lex_open: cannot open file `%s' (Not a regular file)",
                   fname);
        return NULL;
    }

    /* Open the stream */
    if (NULL==(stream=fopen(fname, "r"))) {
#ifdef HAVE_STRERROR
        out_errorn("lex_open: cannot open file `%s' (%s)",
                   fname, strerror(errno));
#else
        out_errorn("lex_open: cannot open file `%s' (errno=%d)",
                   fname, errno);
#endif
        return NULL;
    }

    /* Create the lex file pointer */
    f = calloc(1, sizeof(lex_t));
    assert(f);
    f->f = stream;
    f->prompt = LEX_PROMPT;
    if (!LEX_STDIN) LEX_STDIN = f;
    return f;
}

/*-------------------------------------------------------------------------
 * Function:    lex_stream
 *
 * Purpose:     Reopens a stream for reading.
 *
 * Return:      Success:        Ptr to a lex_t input file.
 *
 *              Failure:        NULL, error printed.
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-07-10
 *              Sets LEX_STDIN if unset.
 *
 *      Thomas R. Treadway, Tue Jun 27 13:59:21 PDT 2006
 *      Added HAVE_STRERROR wrapper
 *-------------------------------------------------------------------------
 */
lex_t *
lex_stream(FILE *stream)
{
    lex_t        *f = calloc(1, sizeof(lex_t));

    assert (f);
    if (NULL==(f->f=fdopen(fileno(stream), "r"))) {
#ifdef HAVE_STRERROR
        out_errorn ("lex_stream: cannot reopen stream (%s)",
                    strerror(errno));
#else
        out_errorn ("lex_stream: cannot reopen stream (errno=%d)", errno);
#endif
        free(f);
        return NULL;
    }
    f->prompt = LEX_PROMPT;
    if (!LEX_STDIN) LEX_STDIN = f;
    return f;
}

/*-------------------------------------------------------------------------
 * Function:    lex_string
 *
 * Purpose:     Creates a lexer input object which reads a buffer supplied
 *              by the caller.
 *
 * Return:      Success:        Ptr to a new lex_t input object.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *              Jeremy Meredith, Thu Aug 26 09:59:44 PDT 1999
 *              Changed use of strdup() to safe_strdup().
 *
 *              Robb Matzke, 2000-07-10
 *              Sets LEX_STDIN if unset.
 *-------------------------------------------------------------------------
 */
lex_t *
lex_string(const char *s)
{
    lex_t        *f = calloc(1, sizeof(lex_t));

    assert(f);
    f->s = safe_strdup(s?s:"");
    f->prompt = LEX_PROMPT;
    if (!LEX_STDIN) LEX_STDIN = f;
    return f;
}

/*---------------------------------------------------------------------------
 * Purpose:     Create an input object which is a sequence of other input
 *              objects.
 *
 * Return:      New stack input object.
 *
 * Programmer:  Robb Matzke
 *              Monday, July 10, 2000
 *
 * Modifications:
 *-----------------------------------------------------------------------------
 */
lex_t *
lex_stack(void)
{
    lex_t       *f = calloc(1, sizeof(lex_t));
    if (!LEX_STDIN) LEX_STDIN = f;
    return f;
}

/*-----------------------------------------------------------------------------
 * Purpose:     Push a new input item onto the stack.
 *
 * Programmer:  Robb Matzke
 *              Monday, July 10, 2000
 *
 * Modifications:
 *-----------------------------------------------------------------------------
 */
void
lex_push(lex_t *f, lex_t *item)
{
    if (f->nstack+1>=NELMTS(f->stack)) {
        out_errorn("file inclusion nested too deeply");
    } else {
        f->stack[f->nstack++] = item;
    }
}

/*-------------------------------------------------------------------------
 * Function:    lex_close
 *
 * Purpose:     Closes a lexer input file.
 *
 * Return:      Success:        NULL
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-07-10
 *              Modified to work with stack input items.
 *-------------------------------------------------------------------------
 */
lex_t *
lex_close(lex_t *f)
{
    int         i;

    assert(f);

    if (f==LEX_STDIN) LEX_STDIN=NULL;

    if (f->f) fclose(f->f);
    if (f->s) free(f->s);
    for (i=0; i<f->nstack; i++) lex_close(f->stack[i]);
    memset(f, 0, sizeof(lex_t));
    free(f);
    return NULL;
}

/*-------------------------------------------------------------------------
 * Function:    lex_getc
 *
 * Purpose:     Similar to getc(3) except uses the GNU readline library
 *              and issues prompts as necessary.
 *
 * Return:      Success:        Next character
 *
 *              Failure:        EOF
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *      Robb Matzke, 29 Jul 1997
 *      If the line-feed is escaped with a backslash, then the backslash
 *      and line-feed are both ignored.
 *
 *      Jeremy Meredith, Thu Aug 26 09:59:44 PDT 1999
 *      Changed use of strdup() to safe_strdup().
 *
 *      Robb Matzke, 2000-07-10
 *      Modified to work with stacked input streams.
 *-------------------------------------------------------------------------
 */
int
lex_getc(lex_t *f)
{
    int          c=EOF;
#ifdef HAVE_READLINE_HISTORY
    static char  buf[1024];
#endif

    if (f->s) {
        c = f->s[f->at++];
        if (!f->s[f->at]) {
            free(f->s);
            f->s = NULL;
            f->at = 0;
        }

    } else if (f->f && isatty(fileno(f->f))) {
        /* Input is from the standard input stream.  Use readline() to
         * get it and add it to the history if different than the
         * previous line. */
#if defined(HAVE_READLINE_READLINE_H) && defined(HAVE_LIBREADLINE)
        char *temp = readline(f->prompt);
        if (temp) {
            f->s = malloc(strlen(temp)+2);
            strcpy(f->s, temp);
            strcat(f->s, "\n");
        }
#else
        char temp[4096];
        fputs(f->prompt, stdout);
        if (fgets(temp, sizeof(temp), f->f)) {
            f->s = safe_strdup(temp);
        } else {
            f->s = NULL;
        }
#endif
        f->at = 0;
#if defined(HAVE_READLINE_READLINE_H) && defined(HAVE_READLINE_HISTORY)
        if (f->s && f->s[0] && strncmp(buf, f->s, sizeof(buf))) {
            add_history(f->s);
            strncpy(buf, f->s, sizeof(buf));
        }
#endif
        c = (f->s ? lex_getc(f) : EOF);

    } else if (f->f) {
        /* Input is from a non-interactive stream. */        
        c = getc(f->f);
        
    } else if (f->nstack) {
        while (f->nstack && EOF==(c=lex_getc(f->stack[f->nstack-1]))) {
            lex_close(f->stack[f->nstack-1]);
            f->stack[--f->nstack] = NULL;
        }
        return c;
   
    } else {
        return EOF;
    }

    /* If this character is a backslash and the following character
     * is a line-feed, then ignore both of them and return the following
     * character instead.  This allows us to always continue a line by
     * escaping the line-feed. */
    if ('\\'==c) {
        int peek = lex_getc(f);
        if ('\n'!=peek) lex_ungetc(f, peek);
        else c = lex_getc(f);
    }

    return c;
}

/*-------------------------------------------------------------------------
 * Function:    lex_ungetc
 *
 * Purpose:     Pushes a character back onto the input stream.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *              Jeremy Meredith, Thu Aug 26 09:59:44 PDT 1999
 *              Changed use of strdup() to safe_strdup().
 *
 *              Robb Matzke, 2000-07-10
 *              Modified to work with stacked input.
 *-------------------------------------------------------------------------
 */
int
lex_ungetc(lex_t *f, int c)
{
    int          status = (-1);

    if (EOF==c) {
        status = -1;

    } else if (f->s) {
        /* Input is from a string or the GNU readline library.  Just
         * back up the string offset. */
        if (f->at>0 && f->s[f->at-1]==c) {
            f->at -= 1;
            status = 0;
        } else {
            status = -1;
        }
        
    } else if (f->f && !isatty(fileno(f->f))) {
        /* Input is from a file. */        
        status = ungetc(c, f->f);

    } else {
        /* Allocate a buffer for the pushback */
        f->s = malloc(2);
        f->s[0] = c;
        f->s[1] = '\0';
        f->at = 0;
    }
    return status;
}

/*-------------------------------------------------------------------------
 * Function:    lex_gets
 *
 * Purpose:     Like fgets(3) except using the lexer input stream.  The
 *              current token (if any) is not affected.
 *
 * Return:      Success:        BUF
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 10 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lex_gets(lex_t *f, char *buf, int size)
{
    int          c, at=0;

    while (EOF!=(c=lex_getc(f)) && '\n'!=c) {
        if (at+1<size) buf[at++] = c;
    }
    if (size>0) buf[at] = '\0';
    return (EOF!=c || at>0) ? buf : NULL;
}

/*-------------------------------------------------------------------------
 * Function:    lex_token
 *
 * Purpose:     Figures out what token is next on the input stream.  If
 *              skipnl is non-zero then the new-line token is skipped.
 *
 * Return:      Success:        Token number, optional lexeme returned
 *                              through the LEXEME argument.
 *
 *              Failure:        TOK_INVALID
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Cleaned up error messages.
 *
 *      Robb Matzke, 7 Feb 1997
 *      Added the `=' token.
 *
 *      Robb Matzke, 7 Feb 1997
 *      The `*' and `?'characters are now legal as part of a symbol name
 *      so we can give those pattern matching characters to the `ls'
 *      command.
 *
 *      Robb Matzke, 12 Mar 1997
 *      Since we don't have mathematical expressions yet, a numeric
 *      constant is allowed to begin with a `-'.
 *
 *      Robb Matzke, 2000-06-06
 *      Symbol names may include `-'. Something that starts with a `-' is
 *      only a number if it's followed by a digit.
 *
 *      Mark C. Miller, Mon Nov  9 18:08:05 PST 2009
 *      Added logic to support parsing of '#nnnnnn' dataset names,
 *      but only when in '/.silo' dir.
 *-------------------------------------------------------------------------
 */
int
lex_token(lex_t *f, char **lexeme, int skipnl)
{
    int          c, at, quote, inDotSiloDir=0;
    static const char *symcharsA = "_$/*?";
    static const char *symcharsB = "_$/*?#";
    const char *symchars = symcharsA;

    /* Return the current token if appropriate. */    
    if (f->tok && (!skipnl || TOK_EOL!=f->tok)) {
        if (lexeme) *lexeme = f->lexeme;
        return f->tok;
    }

    /* Skip leading space. */    
    f->prompt = skipnl ? LEX_PROMPT2 : LEX_PROMPT;
    while (EOF!=(c=lex_getc(f)) && '\n'!=c && isspace(c)) /*void*/;

    /* handle special case of leading '#' and see if we're in .silo dir */
    if ('#'==c) {
        obj_t   f1, val;
        DBfile *file;
        char cwd[1024];

        f1 = obj_new (C_SYM, "$1");
        val = sym_vboundp (f1);
        f1 = obj_dest (f1);
        if (NULL!=(file=file_file(val)) && 
            DBGetDir(file, cwd)>=0 &&
            !strncmp(cwd,"/.silo",6)) {
            inDotSiloDir = 1;
            symchars = symcharsB;
        }
    }

    /* Store the next token. */    
    if (EOF==c) {
        f->lexeme[0] = '\0';
        f->tok = EOF;

    } else if ('\n'==c) {
        if (skipnl) {
            f->tok = lex_token(f, NULL, true);
        } else {
            f->lexeme[0] = '\n';
            f->lexeme[1] = '\0';
            f->tok = TOK_EOL;
        }

    } else if ('#'==c && !inDotSiloDir) {
        while (EOF!=(c=lex_getc(f)) && '\n'!=c) /*void*/;
        lex_ungetc(f, c);
        return lex_token(f, lexeme, skipnl);

    } else if ('>'==c) {
        c = lex_getc(f);
        if ('>'==c) {
            strcpy(f->lexeme, ">>");
            f->tok = TOK_RTRT;
        } else {
            lex_ungetc(f, c);
            strcpy(f->lexeme, ">");
            f->tok = TOK_RT;
        }

    } else if (strchr("|.()[]{}:,=", c)) {
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        f->tok = c;

    } else if (isalpha(c) || strchr(symchars,c)) {
        /* A symbol. */        
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        at = 1;
        while (EOF!=(c=lex_getc(f)) &&
               (isalpha(c) || isdigit(c) || strchr(symchars, c))) {
            if (at+1<sizeof(f->lexeme)) {
                f->lexeme[at++] = c;
                f->lexeme[at] = '\0';
            }
        }
        lex_ungetc(f, c);
        f->tok = TOK_SYM;
      
    } else if ('-'==c) {
        /* Could be a number or a symbol */
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        if (EOF!=(c=lex_getc(f)) && ('.'==c || isdigit(c))) {
            f->lexeme[1] = c;
            f->lexeme[2] = '\0';
            at = 2;
            while (EOF!=(c=lex_getc(f)) &&
                   (isdigit(c) || strchr("+-.eE", c))) {
                if (at+1<sizeof(f->lexeme)) {
                    f->lexeme[at++] = c;
                    f->lexeme[at] = '\0';
                }
            }
            lex_ungetc(f, c);
            f->tok = TOK_NUM;
        } else {
            at=1;
            while (EOF!=c &&
                   (isalpha(c) || isdigit(c) || strchr("_$/*?-", c))) {
                if (at+1<sizeof(f->lexeme)) {
                    f->lexeme[at++] = c;
                    f->lexeme[at] = '\0';
                }
                c = lex_getc(f);
            }
            lex_ungetc(f, c);
            f->tok = TOK_SYM;
        }
       
    } else if ('-'==c || isdigit(c)) {
        /* A number */        
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        at = 1;
        while (EOF!=(c=lex_getc(f)) &&
               (isdigit(c) || strchr("+-.eE", c))) {
            if (at+1<sizeof(f->lexeme)) {
                f->lexeme[at++] = c;
                f->lexeme[at] = '\0';
            }
        }
        lex_ungetc(f, c);
        f->tok = TOK_NUM;

    } else if ('"'==c || '\''==c) {
        /* A string */        
        quote = c;
        at = 0;
        f->lexeme[0] = '\0';
        while (EOF!=(c=lex_getc(f)) && quote!=c && '\n'!=c) {
            if ('\\'==c) {
                switch ((c=lex_getc(f))) {
                case 'b':
                    c = '\b';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case EOF:
                    c = '\\';
                    break;
                default:
                    if (c>='0' && c<='7') {
                        int c2 = lex_getc(f);
                        if (c2>='0' && c2<='7') {
                            int c3 = lex_getc(f);
                            if (c3>='0' && c3<='7') {
                                c = ((c-'0')*8+c2-'0')*8+c3-'0';
                            } else {
                                lex_ungetc(f, c3);
                                c = (c-'0')*8+c2-'0';
                            }
                        } else {
                            lex_ungetc(f, c2);
                            c -= '0';
                        }
                    }
                    break;
                }
            }
            if (at+1<sizeof(f->lexeme)) {
                f->lexeme[at++] = c;
                f->lexeme[at] = '\0';
            }
        }
        if ('\n'==c) {
            out_errorn("linefeed inside string constant (truncated at EOL)");
            lex_ungetc(f, c);
        } else if (c<0) {
            out_errorn("EOF inside string constant (truncated at EOF)");
        }
        f->tok = TOK_STR;

    } else {
        /* Invalid character.  Don't print an error message since a
         * syntax error will result in the parser anyway. */
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        f->tok = TOK_INVALID;
    }

    if (lexeme) *lexeme = f->lexeme;
    return f->tok;
}

/*-------------------------------------------------------------------------
 * Function:    lex_special
 *
 * Purpose:     Special parsing for the next token.  For instance, the next
 *              token might be an unquoted file name `file.pdb' which would
 *              normally be returned as (SYM DOT SYM).  Instead, this function
 *              would parse it as a single string.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
lex_special(lex_t *f, int skipnl)
{
    int          c, at=0;

    if (skipnl && f->tok) {
        f->tok = 0;
        f->lexeme[0] = 0;
    }

    assert(0==f->tok);          /*too late for special lexical analysis*/

    /* Skip leading space.  Skip line-feeds too if SKIPNL is non-zero. */    
    f->prompt = skipnl ? LEX_PROMPT2 : LEX_PROMPT;
    while (EOF!=(c=lex_getc(f)) && isspace(c) && (skipnl || '\n'!=c)) /*void*/;
    if (EOF==c) return;

    if (isalpha(c) || isdigit(c) || strchr ("!@$%^&*-_=+,.?/;:~", c)) {
        f->lexeme[0] = c;
        f->lexeme[1] = '\0';
        at = 1;
        while (EOF!=(c=lex_getc(f)) &&
               (isalpha(c) || isdigit(c) || strchr("!@$%^&*-_=+,.?/;:~", c))) {
            if (at+1<sizeof(f->lexeme)) {
                f->lexeme[at++] = c;
                f->lexeme[at] = '\0';
            }
        }
        f->tok = TOK_STR;
    }
    lex_ungetc (f, c);
}

/*-------------------------------------------------------------------------
 * Function:    lex_consume
 *
 * Purpose:     Consumes the current token.
 *
 * Return:      Success:        Token that was consumed.
 *
 *              Failure:        EOF
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lex_consume(lex_t *f)
{
    int          retval;

    retval = lex_token (f, NULL, false);
    f->tok = 0;
    return retval;
}

/*-------------------------------------------------------------------------
 * Function:    lex_set
 *
 * Purpose:     Sets the current token and lexeme to that which is
 *              specified.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
lex_set(lex_t *f, int tok, char *lexeme)
{
    f->tok = tok;
    if (lexeme) strcpy (f->lexeme, lexeme);
    else f->lexeme[0] = '\0';
}

/*-------------------------------------------------------------------------
 * Function:    lex_strtok
 *
 * Purpose:     Same as strtok(3) except it takes an extra argument and
 *              it's reentrant.
 *
 * Return:      Success:        Ptr to next token.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.com
 *              Jul 30 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lex_strtok(char *s, const char *delim, strtok_t *state)
{
    if (!s) {
        /* Continue from a previous time... */        
        if (state->stop) {
            *(state->stop) = state->save;
            s = state->stop + 1;
        }
    }


    if (s) {
        /* Get the next token. */        
        s += strspn(s, delim);
        state->stop = strpbrk(s, delim);
        if (state->stop) {
            state->save = *(state->stop);
            *(state->stop) = '\0';
        }
    }

    return s;
}

