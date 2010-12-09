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
 * Created:             output.c
 *                      Dec 11 1996
 *                      Robb Matzke <robb@maya.nuance.mdn.com>
 *
 * Purpose:             Output functions.
 *
 * Modifications:       
 *              Robb Matzke, 2000-06-01
 *              Added const qualifiers to formal arguments.
 *
 *              Thomas R. Treadway, Tue Jun 27 14:19:57 PDT 2006
 *              Added config.h for HAVE_STRERROR wrapper
 *-------------------------------------------------------------------------
 */
#include "config.h"
#include <assert.h>
#include <browser.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#ifndef _WIN32
#  include <termios.h>
#  include <sys/ioctl.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#define         OUT_INDENT 3            /*chars per indentation level   */
#define         OUT_RTMAR 2             /*right margin                  */
int             OUT_NROWS=0;            /*lines per screen, or zero     */
int             OUT_NCOLS=80;           /*columns per screen, or zero   */
int             OUT_LTMAR=20;           /*left margin                   */
int             OUT_COL2=50;            /*beginning of second column    */
static int      ErrorDisable=0;         /*disable error messages        */
static int      BrokenPipe=0;           /*incremented by SIGPIPE        */
static int      CaughtSigint=0;         /*incremented by SIGINT         */
static int      Progress=0;             /*current progress report size  */

out_t           *OUT_STDOUT;            /*standard output               */
out_t           *OUT_STDERR;            /*standard error                */


/*-------------------------------------------------------------------------
 * Function:    handle_signals
 *
 * Purpose:     Gets called for SIGPIPE and SIGINT signals.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 13 1997
 *
 * Modifications:
 *   Kathleen Bonnell, Thu Dec 9 09:33:15 PST 2010\
 *   SIGPIPE is not defined on WIN32.
 *-------------------------------------------------------------------------
 */
static void
handle_signals(int signo)
{
    switch (signo) {
#ifndef _WIN32
    case SIGPIPE:
        BrokenPipe++;
        break;
#endif

    case SIGINT:
        CaughtSigint++;
        break;
    }
}


/*-------------------------------------------------------------------------
 * Function:    out_brokenpipe
 *
 * Purpose:     Determines if output needs to be disabled for some
 *              reason.  Output is disabled if we've received SIGPIPE or
 *              SIGINT or the user has set some non-default pager mode for
 *              the specified file.
 *
 *              Pending SIGINT and SIGPIPE signals are assigned to the
 *              specified file as a side effect.
 *
 * Returns:     Returns zero (PAGER_OKAY) if output can continue as
 *              normal. Returns one of the non-zero PAGER_* constants
 *              otherwise.
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 13 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
pflags_t
out_brokenpipe(out_t *f)
{
    /* Assign pending signals to this file */
    if (BrokenPipe) {
        BrokenPipe = 0;
        f->pflags = PAGER_PIPE;
    } else if (CaughtSigint) {
        CaughtSigint = 0;
        f->pflags = PAGER_INTERRUPT;
    }

    /* PAGER_NEXT_CMD does not turn off output, but rather temporarily
     * disables the pager itself. */
    if (PAGER_NEXT_CMD==f->pflags) return PAGER_OKAY;

    /* Pipe status */
    return f->pflags;
}


/*-------------------------------------------------------------------------
 * Function:    out_section
 *
 * Purpose:     Resets pager to handle a new section of output from the
 *              current command. This is used by the `diff' command (for
 *              example) so that the user can skip from one variable to
 *              the next without wading through lots of gunk.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke, 2001-02-09
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
void
out_section(out_t *f)
{
    if (PAGER_NEXT_SECTION==f->pflags) f->pflags = PAGER_OKAY;
}


/*-------------------------------------------------------------------------
 * Function:    out_error
 *
 * Purpose:     Prints an error message followed by an object.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Changed prefix name from `error' to `***ERROR***' to make it
 *      stand out more.
 *
 *-------------------------------------------------------------------------
 */
void
out_error (const char *mesg, obj_t obj) {

   if (!ErrorDisable) {
      out_reset (OUT_STDERR);
      out_push (OUT_STDERR, "***ERROR***");
      out_putw (OUT_STDERR, mesg);
      obj_print (obj, OUT_STDERR);
      out_pop (OUT_STDERR);
      out_nl (OUT_STDERR);
   }
}


/*-------------------------------------------------------------------------
 * Function:    out_errorn
 *
 * Purpose:     Print an error message to the standard error stream,
 *              followed by a linefeed.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Changed prefix name from `error' to `***ERROR***' to make it
 *      stand out more.
 *
 *-------------------------------------------------------------------------
 */
void
out_errorn (const char *fmt, ...) {

   char         buf[4096];
   va_list      ap;

   if (!ErrorDisable) {
      va_start (ap, fmt);
      vsprintf (buf, fmt, ap);
      va_end (ap);

      if (OUT_STDERR && OUT_STDERR->f) {
         out_reset (OUT_STDERR);
         out_push (OUT_STDERR, "***ERROR***");
         out_putw (OUT_STDERR, buf);
         out_pop (OUT_STDERR);
         out_nl (OUT_STDERR);
      } else {
         fputs (buf, stderr);
         fputc ('\n', stderr);
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:    out_error_disable
 *
 * Purpose:     Disables error messages.
 *
 * Return:      Success:        0
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
out_error_disable (void) {

   ErrorDisable++;
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:    out_error_restore
 *
 * Purpose:     Restores previous error reporting state.
 *
 * Return:      Success:        0 if errors are re-enabled, positive if
 *                              errors are still disabled because of
 *                              other outstanding calls to out_error_disable().
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
out_error_restore (void) {

   if (ErrorDisable>0) {
      --ErrorDisable;
   } else {
      return -1;
   }
   return ErrorDisable;
}


/*-------------------------------------------------------------------------
 * Function:    out_indent
 *
 * Purpose:     Increase the indentation level by one.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
out_indent (out_t *f) {

   f->indent += 1;
}


/*-------------------------------------------------------------------------
 * Function:    out_info
 *
 * Purpose:     Prints an informational message.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Informational messages are just printed to the standard error
 *      stream with no formatting except adding the word `INFO: ' to
 *      the beginning and adding a trailing linefeed.
 *
 *      Robb Matzke, 4 Feb 1997
 *      If the message starts with `WARNING: ' or `INFO: ' then `INFO: '
 *      is not printed before the message.
 *
 *-------------------------------------------------------------------------
 */
void
out_info (const char *fmt, ...) {

   char         buf[4096];
   int          n;
   va_list      ap;

   out_progress (NULL);
   va_start (ap, fmt);
   vsprintf (buf, fmt, ap);
   va_end (ap);

   if (strncmp(buf, "INFO: ", 6) && strncmp(buf, "WARNING: ", 9)) {
      fputs ("INFO: ", stderr);
   }
   fputs (buf, stderr);
   n = strlen (buf);
   if (0==n || '\n'!=buf[n-1]) fputc ('\n', stderr);
}

/*---------------------------------------------------------------------------
 * Purpose:     Gets the height and width of the output tty.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  6, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
out_init_size(void)
{
    int         width = 80;             /*the default                   */
    int         height = 0;             /*the default is no paging info */
    const char  *s;

    /*
     * Try to get it from the COLUMNS environment variable first since it's
     * value is sometimes wrong.
     */
    if ((s=getenv("COLUMNS")) && isdigit(*s)) {
        width = strtol(s, NULL, 0);
    }
    if ((s=getenv("LINES")) && isdigit(*s)) {
        height = strtol(s, NULL, 0);
    }
#if defined(TIOCGWINSZ)
    {
        /* Unix with ioctl(TIOCGWINSZ) */
        struct winsize w;
        if (ioctl(2, TIOCGWINSZ, &w)>=0 && w.ws_col>0) {
            width = w.ws_col;
            height = w.ws_row;
        }
    }
#elif defined(TIOCGETD)
    {
        /* Unix with ioctl(TIOCGETD) */
        struct uwdata w;
        if (ioctl(2, WIOCGETD, &w)>=0 && w.uw_width>0) {
            width = w.uw_width / w.uw_hs;
            height = w.uw_height / w.uw_vs; /*is this right? -rpm*/
        }
    }
#endif
    
    /* Set width to at least 1, height to at least zero */
    if (width<1) width = 1;
    if (height<0) height = 0;

    /* Set the global values */
    OUT_NCOLS = width;
    OUT_NROWS = height;
    OUT_LTMAR = 0.25 * OUT_NCOLS;
    OUT_COL2 = (OUT_LTMAR+OUT_NCOLS)/2;
}


/*-------------------------------------------------------------------------
 * Function:    out_init
 *
 * Purpose:     Initialize output.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-05-23
 *              The column in which the equal sign appears is 25 percent
 *              of the way across the screen.
 *
 *              Robb Matzke, 2000-06-01
 *              The window size is obtained by ioctl() when possible.
 *
 *              Robb Matzke, 2000-06-28
 *              Signal handlers are registered with sigaction() since its
 *              behavior is more consistent.
 *
 *              Kathleen Bonnell, Thu Dec 9 09:34:51 PST 2010
 *              sigaction not defined on Win32.
 *-------------------------------------------------------------------------
 */
void
out_init (void)
{
#ifndef _WIN32
    struct sigaction    action;
#endif
    
    /* Keep track of terminal size changes */
    out_init_size();
    
    /* Open standard streams */
    OUT_STDOUT = out_stream (stdout);
    OUT_STDERR = out_stream (stderr);

    /* Arrange to handle broken pipes and interrupts */
#ifndef _WIN32
    action.sa_handler = handle_signals;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &action, NULL);
    sigaction(SIGINT, &action, NULL);
#endif
}


/*-------------------------------------------------------------------------
 * Function:    out_literal
 *
 * Purpose:     Turns on or off literal output.  When literal mode is
 *              on, no line splitting or white-space eating occurs.
 *
 * Return:      Success:        Old literal setting
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
out_literal (out_t *f, int val) {

   int          old_literal = f->literal;

   f->literal = val;
   return old_literal;
}
   

/*-------------------------------------------------------------------------
 * Function:    out_nl
 *
 * Purpose:     Ends the current line.  The new line is not started until
 *              output occurs on the new line.  If writing a linefeed would
 *              cause information to scroll off the top of a terminal, then
 *              the output is paused.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 23 Jan 1997
 *      The `more?' prompt has been modified to be more interactive
 *      by placing the terminal in raw mode.
 *
 *      Robb Matzke, 6 Feb 1997
 *      Lines which would have nothing after the equal are left blank.
 *
 *      Kathleen Bonnell, Thu Dec 9 09:34:27 PST 2010
 *      Comment out the paging functionality on Win32. (Till a path forward
 *      for this platform can be determined).
 *-------------------------------------------------------------------------
 */
void
out_nl(out_t *f)
{
#ifndef _WIN32
    char                buf[256];
    int                 i, n, rawmode=false;
    struct termios      oldtio, tio;
    static const char   *prompt = "more? ('q' to quit) ";
#endif

    if (out_brokenpipe(f)) return;
    if (isatty(fileno(f->f))) out_progress(NULL);

    putc('\n', f->f);
    f->row += 1;
    f->col = 0;
#ifdef _WIN32
    fflush(f->f);
#else
    /* Pause output if it's going to a terminal. */    
 again:
    if (PAGER_ACTIVE(f) && f->row+1==OUT_NROWS) {
        fputs(prompt, f->f);
        fflush(f->f);

        if (tcgetattr(STDIN_FILENO, &tio)>=0) {
            oldtio = tio;
            tio.c_lflag &= ~(ECHO | ICANON);
            tio.c_lflag &= ~ISIG; /*we handle them below*/
            tio.c_cc[VMIN] = 1;
            tio.c_cc[VTIME] = 0;
            if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio)>=0) rawmode=true;
        }

        while (0==out_brokenpipe(f) &&
               (n=read(STDIN_FILENO, &buf, 1))<0 &&
               EINTR==errno) /*void*/;
      
        if (rawmode) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldtio);
            for (i=0; i<strlen(prompt); i++) putc('\b', f->f);
            for (i=0; i<strlen(prompt); i++) putc(' ', f->f);
            for (i=0; i<strlen(prompt); i++) putc('\b', f->f);
            fflush(f->f);
        }
        if (1==n && 'q'==buf[0]) handle_signals(SIGPIPE);
        if (1==n && 'n'==buf[0]) {
            f->pflags = PAGER_NEXT_SECTION;
            fputs("Skipping...\n", f->f);
        }
        if (1==n && tio.c_cc[VEOF]==buf[0]) f->pflags = PAGER_NEXT_CMD;
        if (1==n && tio.c_cc[VINTR]==buf[0]) handle_signals(SIGINT);
        if (1==n && tio.c_cc[VQUIT]==buf[0]) kill(getpid(), SIGQUIT);
        if (1==n && tio.c_cc[VSUSP]==buf[0]) {
            kill(getpid(), SIGTSTP);
            goto again;
        }
        f->row = 0;
    }
#endif
}


/*-------------------------------------------------------------------------
 * Function:    out_pop
 *
 * Purpose:     Pop the top-most field record from the output prefix
 *              specifier.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
out_pop (out_t *f) {

   int          n;

   assert(f->nfields>0);
   if (f->nfields>0) {
      f->nfields -= 1;
      n = f->nfields;

      if (f->field[n].name) {
         free (f->field[n].name);
         f->field[n].name = NULL;
      }
   }
}

/*---------------------------------------------------------------------------
 * Description: Similar to out_line() but the output doesn't occur until
 *              the next line of output happens. Calling this function
 *              with an empty string produces one line of output. Calling
 *              this function with the null pointer cancels the header.
 *              Headers are typically used above tabular output when it
 *              isn't known whether any output would occur.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 28, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
out_header(out_t *f, const char *header)
{
    if (f->header) free(f->header);
    f->header = safe_strdup(header);
}


/*-------------------------------------------------------------------------
 * Function:    out_prefix
 *
 * Purpose:     Prints the prefix which appears at the left of every line
 *              of output.  The prefix is printed only if the current
 *              column number is zero.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Added the `silent' attribute to field descriptors.
 *
 *      Robb Matzke, 3 Feb 1997
 *      If there is not prefix, then we print just space left of the
 *      equal sign.
 *
 *      Robb Matzke, 4 Feb 1997
 *      If two array prefix areas are adjacent then we combine them.
 *
 *      Robb Matzke, 2000-06-28
 *      Prints table headers if any.
 *-------------------------------------------------------------------------
 */
void
out_prefix (out_t *f) {

   int          i, j, k, n, stride;
   int          in_array=false;
   char         buf[256];

   if (0==f->col && !out_brokenpipe(f)) {
      if (isatty (fileno (f->f))) out_progress (NULL);

      /* Print table headers if any */
      if (f->header) {
          char *header = f->header;
          f->header = NULL;
          out_line(f, header);
          free(header);
      }
      
      /*
       * Print the field names separated from one another by a dot.
       */
      for (i=0; i<f->nfields; i++) {
         if (f->field[i].silent) continue;
         
         /*
          * The field name.
          */
         if (f->field[i].name) {
            if (in_array) {
               putc (']', f->f);
               f->col += 1;
               in_array = false;
            }
            if (f->col) {
               putc ('.', f->f);
               f->col += 1;
            }
            fputs (f->field[i].name, f->f);
            f->col += strlen (f->field[i].name);
         }
         
         /*
          * Array indices.
          */
         if (f->field[i].ndims>0) {
            if (in_array) {
               fputs (", ", f->f);
               f->col += 2;
            } else {
               putc ('[', f->f);
               f->col += 1;
               in_array = true;
            }
            n = f->field[i].elmtno;
            for (j=0; j<f->field[i].ndims; j++) {
               if (j) {
                  fputs (", ", f->f);
                  f->col += 2;
               }
               for (k=j+1,stride=1; k<f->field[i].ndims; k++) {
                  stride *= f->field[i].dim[k];
               }
               sprintf (buf, "%d", f->field[i].offset[j]+n/stride);
               n %= stride;
               fputs (buf, f->f);
               f->col += strlen(buf);
            }
         }
      }

      if (in_array) {
         putc (']', f->f);
         f->col += 1;
         in_array = false;
      }
      
      /*
       * Print the equal sign so the value starts not earlier than
       * the left margin.
       */
      if (f->col+1>=OUT_LTMAR) {
         fputs ("=", f->f);
         f->col += 1;
      } else {
         fprintf (f->f, "%*s= ", OUT_LTMAR-(f->col+2), "");
         f->col = OUT_LTMAR;
      }

      /*
       * Print the indentation
       */
      n = OUT_LTMAR + f->indent*OUT_INDENT - f->col;
      if (n>0) {
         fprintf (f->f, "%*s", n, "");
         f->col += n;
      }
   }
}

/*---------------------------------------------------------------------------
 * Description: Indent to at least the specified column.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June 27, 2000
 *
 * Modifications:
 *---------------------------------------------------------------------------
 */
void
out_column(out_t *f, int column, const char *separator)
{
    int oldlit = out_literal(f, true);
    
    while (f->col<column && !out_brokenpipe(f)) out_puts(f, " ");
    out_puts(f, separator);
    out_literal(f, oldlit);
}


/*-------------------------------------------------------------------------
 * Function:    out_printf
 *
 * Purpose:     Formatted output.  The entire output is printed on the
 *              current line if it fits.  Otherwise the entire output is
 *              printed on the next line.  Linefeeds within the output
 *              string force line breaks.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
out_printf (out_t *f, const char *fmt, ...) {

   va_list      ap;
   char         buf[4096], *s, *nextline;

   va_start (ap, fmt);
   vsprintf (buf, fmt, ap);
   va_end (ap);

   for (s=buf; s; s=nextline) {
      nextline = strchr (s, '\n');
      if (nextline) *nextline = '\0';
      out_puts (f, s);
      if (nextline) {
         *nextline++ = '\n';
         out_nl (f);
      }
   }
}


/*-------------------------------------------------------------------------
 * Function:    out_push
 *
 * Purpose:     Push another name onto the field list.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
void
out_push (out_t *f, const char *name) {

   if (f->nfields>=OUT_NFIELDS) return;

   memset (f->field + f->nfields, 0, sizeof(f->field[0]));
   f->field[f->nfields].name = name ? safe_strdup (name) : NULL;
   f->field[f->nfields].silent = false;
   f->nfields += 1;
}


/*-------------------------------------------------------------------------
 * Function:    out_push_array
 *
 * Purpose:     Push another array onto the field list.
 *
 * Return:      Success:        Ptr to an integer that should always be
 *                              kept up to date with the current linear
 *                              array element number.  This value will
 *                              be used by out_prefix() to generate the
 *                              array subscripts.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 17 1996
 *
 * Modifications:
 *
 *    Robb Matzke, 3 Feb 1997
 *    If the `dim' field is NULL then the `silent' attribute is set
 *    for this field.
 *
 *    Lisa J. Roberts, Mon Nov 22 17:27:53 PST 1999
 *    I changed strdup to safe_strdup.
 *
 *-------------------------------------------------------------------------
 */
int *
out_push_array (out_t *f, const char *name, int ndims, const int *offset,
                const int *dim) {

   int          idx, i;

   assert (f);
   assert (ndims>0 && ndims<=NDIMS);
   
   idx = f->nfields;
   if (idx>=OUT_NFIELDS) return NULL;
   f->nfields += 1;

   if (name && *name) {
      f->field[idx].name = safe_strdup (name);
   } else {
      f->field[idx].name = NULL;
   }
   f->field[idx].elmtno = 0;
   f->field[idx].ndims = ndims;

   if (dim) {
      for (i=0; i<ndims; i++) {
         f->field[idx].offset[i] = offset ? offset[i] : 0;
         f->field[idx].dim[i] = dim[i];
      }
      f->field[idx].silent = false;
   } else {
      f->field[idx].silent = true;
   }

   return &(f->field[idx].elmtno);
}
   

/*-------------------------------------------------------------------------
 * Function:    out_puts
 *
 * Purpose:     Prints string S to the specified file.  If S fits on the
 *              current line then S is printed on the current line. Otherwise
 *              S is printed on the next line.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *              Robb Matzke, 17 Dec 1996
 *              If literal mode is on, then no linefeeds are inserted to
 *              break up long lines.
 *
 *              Robb Matzke, 2000-06-27
 *              If the file handle right margin is zero then no linefeeds
 *              are inserted to break up long lines.
 *-------------------------------------------------------------------------
 */
void
out_puts (out_t *f, const char *s) {

   int          n = strlen (s);

   if (isatty (fileno (f->f))) out_progress (NULL);
   if (!f->literal && f->rtmargin && n+f->col>OUT_NCOLS-f->rtmargin) {
       out_nl (f);
   }
   
   if (out_brokenpipe(f)) return;
   if (s && *s) {
      if (!f->literal && 0==f->col) {
         while (*s && isspace(*s)) s++;
      }
      out_prefix (f);
      fputs (s, f->f);
      f->col += n;
   }
}


/*-------------------------------------------------------------------------
 * Function:    out_line
 *
 * Purpose:     Prints S on a line by itself.
 *
 * Return:      Success:        
 *
 *              Failure:        
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 21 1997
 *
 * Modifications:
 *              Robb Matzke, 2000-06-28
 *              Handles embedded linefeeds.
 *-------------------------------------------------------------------------
 */
void
out_line (out_t *f, const char *string)
{
    int         oldlit;
    const char  *s;

    if (out_brokenpipe(f)) return;
    if (isatty (fileno (f->f))) out_progress (NULL);
    oldlit = out_literal (f, true);
    if (f->col) out_nl (f);

    /* The header */
    if (f->header) {
        for (s=f->header; *s; s++) {
            if ('\n'==*s) {
                out_nl(f);
            } else {
                putc(*s, f->f);
                f->col++;
            }
        }
        out_nl(f);
        free(f->header);
        f->header = NULL;
    }
    
    /* The output */
    for (s=string; *s; s++) {
        if ('\n'==*s) {
            out_nl(f);
        } else {
            fputc(*s, f->f);
            f->col++;
        }
    }

    /* The trailing line-feed */
    out_nl(f);
    out_literal (f, oldlit);
}


/*-------------------------------------------------------------------------
 * Function:    out_progress
 *
 * Purpose:     Prints a progress report which is a single line w/o
 *              a trailing line feed.  The other functions in this
 *              file are aware of these progress reports and make sure
 *              that the report is deleted properly when output occurs.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb 18 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
out_progress (const char *s) {

   int          nchars, i=0;
   
   if (Verbosity<1) s = NULL;

   if (isatty (2) && Verbosity<=1) {
      for (nchars=0; nchars<79 && s && s[nchars]; nchars++) {
         fputc (s[nchars]>=' ' && s[nchars]<='~' ? s[nchars] : ' ', stderr);
      }
      for (i=nchars; i<Progress; i++) putc (' ', stderr);
      for (i=nchars; i<Progress; i++) putc ('\b', stderr);
      for (i=0; i<nchars; i++) putc ('\b', stderr);
   } else if (s) {
      fputs (s, stderr);
      putc ('\n', stderr);
   }
   fflush (stderr);
   Progress = i;
}
   

/*-------------------------------------------------------------------------
 * Function:    out_putw
 *
 * Purpose:     Similar to out_puts() except the string can be split on
 *              white space.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Apr 1997
 *      Words can be split on `/', `-', or `_', and those characters appear
 *      at the end of the previous line.
 *
 *      Robb Matzke, 2000-06-02
 *      Only the first line of output gets the prefix.
 *-------------------------------------------------------------------------
 */
void
out_putw (out_t *f, const char *s) {

    const char  *t;
    char        buf[1024];
    int         nl, i;
    int         old_nfields = f->nfields;

    while (s && *s) {
        if (out_brokenpipe(f)) return;

        /* Find the edges of the next word. */
        for (t=s, nl=0; *t && strchr (" \t\n_-/", *t); t++) {
            if ('\n'==*t) nl++; /*skip leading seps, remember newlines*/
        }
        t = strpbrk (t, " \t\n_-/"); /*find next word separator*/
        while (t && *t && strchr("_-/",*t)) t++; /*these stay on current line*/
        if (!t) t = s+strlen(s);

        /* Extract the word. */
        strncpy (buf, s, t-s);
        buf[t-s] = '\0';

        /* Print empty lines */
        for (i=0; i<nl; i++) {
            out_nl(f);
            f->nfields=0;
        }

        /* Output the word including leading white space.  The out_puts()
         * function will strip leading whitespace at the beginning of
         * a line. */
        out_puts (f, buf);
        s = t;

        /* Turn off prefix for subsequent lines */
        f->nfields=0;
    }
    f->nfields = old_nfields; /*restore prefix*/
}


/*-------------------------------------------------------------------------
 * Function:    out_reset
 *
 * Purpose:     Call this function to reset the current line and column
 *              number to zero in preparation for new output.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 20 Jan 1997
 *      Also resets the BrokenPipe and CaughtSigint counters.
 *
 *-------------------------------------------------------------------------
 */
void
out_reset (out_t *f) {

   BrokenPipe = CaughtSigint = 0;
   f->row = f->col = 0;
   out_progress (NULL);
   f->pflags = PAGER_OKAY;
}


/*-------------------------------------------------------------------------
 * Function:    out_stream
 *
 * Purpose:     Duplicates a FILE* for use as an output stream.
 *
 * Return:      Success:        Ptr to the new `out_t' structure.
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *              Thomas R. Treadway, Tue Jun 27 14:19:57 PDT 2006
 *              Added HAVE_STRERROR wrapper
 *
 *-------------------------------------------------------------------------
 */
out_t *
out_stream (FILE *stream) {

   out_t        *f = calloc (1, sizeof(out_t));

   assert (f);
   if (NULL==(f->f=fdopen(fileno(stream), "w"))) {
#ifdef HAVE_STRERROR
      out_errorn ("out_stream: cannot reopen stream (%s)",
                  strerror (errno));
#else
      out_errorn ("out_stream: cannot reopen stream (errno=%d)", errno);
#endif
      free (f);
      return NULL;
   }

   f->paged = isatty(fileno(stream));
   f->rtmargin = OUT_RTMAR;
   return f;
}


/*-------------------------------------------------------------------------
 * Function:    out_undent
 *
 * Purpose:     Decrease the indentation level by one.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Dec 11 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
out_undent (out_t *f) {

   if (f->indent>0) f->indent -= 1;
}


/*-------------------------------------------------------------------------
 * Function:    out_getindex
 *
 * Purpose:     Returns the array index for array N (measured from the
 *              top of the stack if N is negative).  Non-array entries
 *              are not counted.
 *
 *              The first array is zero.  The last array is -1.
 *
 * Return:      Success:        Index number.
 *
 *              Failure:        -1
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan 14 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
out_getindex (out_t *f, int n) {

   int          i;

   if (n>0) {
      for (i=0; i<f->nfields; i++) {
         if (0==f->field[i].ndims) continue;
         if (!n) return f->field[i].offset[0] + f->field[i].elmtno;
         --n;
      }
   } else {
      n++;
      for (i=f->nfields-1; i>=0; --i) {
         if (0==f->field[i].ndims) continue;
         if (!n) return f->field[i].offset[0] + f->field[i].elmtno;
         n++;
      }
   }
   return -1;
}
