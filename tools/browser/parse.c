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
 * Created:             parse.c
 *                      Dec  4 1996
 *                      Robb Matzke <matzke@viper.llnl.gov>
 *
 * Purpose:             Functions for parsing input.
 *
 * Modifications:       
 *
 *-------------------------------------------------------------------------
 */
#include <browser.h>
#include <ctype.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

static void     bif (const char*, obj_t(*)(int,obj_t[]), int, const char*,
                     const char*);
static obj_t    parse_term (lex_t*, int);
static obj_t    parse_dot (lex_t*, int);
static obj_t    parse_selection (lex_t*, int);
static obj_t    parse_expr (lex_t*, int);

static struct obj_t ErrorCell ;         /*error return value    */


/*-------------------------------------------------------------------------
 * Function:    parse_init
 *
 * Purpose:     Initialize the parse system.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 3 Feb 1997
 *      Added commands `end' and `quit' as aliases for `exit'.  Renamed
 *      command `with' as `open'.
 *
 *      Robb Matzke, 2000-06-02
 *      The `help' command doesn't evaluate its arguments.
 *
 *      Robb Matzke, 2000-06-06
 *      Added documentation strings.
 *-------------------------------------------------------------------------
 */
void
parse_init (void) {

   /*** user functions ***/
   bif ("array",        V_array,        0,
        "Create an array datatype.",
        "Creates a new array datatype. The arguments are the "
        "size of the array in each dimension and the datatype of the array "
        "elements. The array size arguments can be either integers or strings "
        "that look like comma- and/or space-separated integers. Browser "
        "arrays use C element ordering.");

   bif ("cd",           V_setcwd,       IMP_STRING,
        "Change current working directory.",
        "Each file has a current working directory (CWD) which is initially "
        "set to the root directory (`/'). This command will set the CWD to "
        "some other directory. The first argument is the name of the new CWD. "
        "If the name contains special characters it should be enclosed in "
        "quotes to protect it from the parser. If the name is relative "
        "(doesn't start with a `/') then the name looked up with respect to "
        "the current CWD.  The second optional argument is the file whose "
        "CWD is being set (default is the file represented by the browser "
        "variable `$1'). If the second argument evaluates to a list of files "
        "then the CWD is set for each file of the list (e.g., `cd domain_0 "
        "$*').");

   bif ("close",        V_close,        HOLD,
        "Close a SILO file.",
        "This function takes zero or more arguments which should all be "
        "symbol names (like `$1') and closes the files represented by each "
        "symbol.  The symbol becomes unbound. Closing the file associated "
        "with symbol FOO is equivalent to saying `FOO=nil', except with "
        "extra sanity checks.");

   bif ("diff",         V_diff,         0,
        "Compare two objects.",
        "Calculates the differences between its arguments similar to the "
        "Unix `diff' command. If called with an even number of arguments "
        "the argument list is split in half and each argument from the first "
        "half is compared with the corresponding argument of the second half. "
        "If called with no arguments then the argument list will be the "
        "SILO files from the command-line ($1, $2, etc).  If called with a "
        "single argument which is a silo object from file $1, that object "
        "will be differenced against an object with the same name from file "
        "$2. The behavior is influenced by the $diff browser variable (see "
        "`help $diff'). In general, if no differences are found then no "
        "output is produced.");

   bif ("end",          V_exit,         0,
        NULL,
        "A synonym for `exit'");
   
   bif ("exit",         V_exit,         0,
        "Exit the browser.", 
        "Causes the browser to exit with zero status. If an integer argument "
        "is supplied then the browser will use it as the exit status. End of "
        "input (usually Control-D for interactive input) causes an implicit "
        "call to `exit 0'.");

   bif ("file",         V_file,         IMP_STRING,
        "Dereference a name to get a SILO file.",
        "Returns the file whose name is specified by the first argument. If "
        "the file name contains special characters then the argument should "
        "be enclosed in quotes to protect it from the parser. The file is "
        "closed as soon as all references to the file dissappear. One common "
        "use of this function is with the `ls' or `diff' functions which can "
        "take file objects as arguments. For instance, the command `ls "
        "(file curv2d.pdb)' opens the file, lists the root directory, and "
        "then closes the file. The file is opened for read-write unless "
        "the file has only read permission or the `$rdonly' variable is "
        "non-zero and non-nil.  See also `help open'.");
        
   bif ("help",         V_help,         HOLDFIRST,
        "Show documentation.",
        "Shows documentation for various parts of the browser. When invoked "
        "without arguments it will display a table of contents of `help' "
        "subcommands. With one argument it will display documentation for a "
        "help category (e.g., `help run'), a browser internal variable (e.g., "
        "`help $diff'), a function (e.g., `help diff'), a command-line switch "
        "(e.g., `help --diff'), an operator (e.g., `help \"op=\"'), or the "
        "documentation string associated with the specified symbol.  When "
        "invoked with a string argument it will print all help categories "
        "which contain the specified string (e.g., `help \"diff\"'). When "
        "called with two arguments it assigns the second argument as the "
        "documentation property of the first argument, which must be a symbol "
        "or string.");

   bif("include",       V_include,      0,
       "Include commands from another source.", 
       "Parses and evaluates the browser commands in the file specified by "
       "the argument, which should evaluate to a string.");
   
   bif ("ls",           V_list,         HOLD,
        "List directory contents.",
        "Lists the current working directory (CWD) of the specified file. If "
        "no file is specified then list the CWD of the file represented by "
        "browser internal variable `$1'. Subsequent arguments are the names "
        "of the objects to be listed. If no names are supplied then all "
        "objects of the CWD are listed. Shell wild-cards are allowed in the "
        "names if the C library supports it.  When `ls' is invoked with an "
        "optional file argument and exactly one name which is the name of "
        "a directory, then the contents of that directory are listed.  If the "
        "first argument evaluates to a list of files (e.g., `ls $*') then a "
        "listing will be generated for each of those files.");

   bif ("noprint",      V_noprint,      0,
        "Supress output.",
        "Takes any number of arguments and returns NIL. Since the browser "
        "doesn't print NIL results by default, this function can be used to "
        "suppress printing of some expression result. It is often used in "
        "startup files.");

   bif ("open",         V_open,         HOLDREST | IMP_STRING,
        "Open a SILO file.",
        "Sets the current file to be the file named by the first argument. "
        "The current file is represented by the browser variable `$1'. If two "
        "arguments are given then the second argument should be a symbol "
        "to which will be bound the SILO file named by the first argument. "
        "If some file was previously bound to the new file's symbol then the "
        "old file is closed first. This function is equivalent to "
        "`arg2=(file \"arg1\")'");

   bif ("pointer",      V_pointer,      0,
        "Create a pointer datatype.",
        "Creates a pointer datatype. The first (and only) argument is the "
        "datatype to which this pointer points.");

   bif ("primitive",    V_primitive,    0,
        "Create a primitive datatype.",
        "Creates a primitive datatype. The first (and only) argument is the "
        "name of the primitive type. Primitive types are: int8, short, int, "
        "long, float, double, and string.");
   
   bif ("print",        V_print,        0,
        "Print an expression.",
        "Takes zero or more arguments and prints all that are non-nil, each "
        "on its own line of output, returning nil. All expressions are "
        "normally printed by the browser in its read-eval-print loop, so "
        "calling this command explicitly is seldom necessary.");
   
   bif ("pwd",          V_pwd,          0,
        "Show the current working directory name.",
        "Prints the current working directory (CWD) of the specified file. If "
        "no file is specified then print the CWD of the file represented by "
        "browser variable `$1'.  If the argument is a list of files then the "
        "CWD of each of those files is printed (e.g., `pwd $*'). This command "
        "is identical to printing an expression that evaluates to a file or "
        "list of files (e.g., `$1' or `$*').");
   
   bif ("quit",         V_exit,         0,
        NULL,
        "A synonym for `exit'.");

   bif ("setf",         V_setf,         HOLDFIRST,
        "Bind a function to a symbol.",
        "Given a symbol as the first argument, sets the symbols functional "
        "value to the second argument.");
   
   bif ("struct",       V_struct,       0,
        "Create a compound datatype.",
        "Creates a compound datatype. The first argument should be the name "
        "for the datatype (or nil). The following arguments occur in triples, "
        "one per compound datatype member. Each triple is an integer byte "
        "offset from the beginning of memory of an instance of this type, the "
        "name of the member, and the datatype of the member.");
   
   bif ("typeof",       V_typeof,       0,
        "Obtain the datatype of an object.",
        "Returns the datatype of a SILO or browser object. For SILO database "
        "objects the type which is printed is fully qualified and bound to "
        "actual values. In contract, printing the actual named type shows the "
        "unbound values. For example, if `d' is a SILO DBquadvar object then "
        "`typeof d.dims' might show `[3] int' while `print DBquadvar.dims' "
        "displays `[self.ndims] int'.");

   /*** Operators ***/
   bif ("Assign",       V_assign,       HOLDFIRST,
        "Assignment operator (`=').",
        "The assignment operator is a binary in-fix operator whose left "
        "operand should be the name of a browser symbol or an object (or "
        "subobject) of a SILO file and whose right operand evaluates to "
        "some value. The value of the right operand is assigned to the "
        "left operand. The `=' operator and `Assign' symbol are bound to "
        "the same function.");

   bif ("Dot",          V_dot,          HOLDREST,
        "Restriction operators (`.', `[]').",
        "This is a restriction operator whose function depends on the "
        "type of the left operand.\n"
        "\n"
        "If the left operand evaluates to a file (e.g., `$1') then the "
        "variable or directory whose name is specified by the second argument "
        "(a string or symbol) is loaded from that file and returned. If "
        "the left operand refers to the same file as `$1' and the right "
        "argument need not be enclosed in quotes and is not the name of "
        "a browser variable, then the file symbol and dot can be omitted "
        "(that is, `$1.quadmesh' is equivalent to just `quadmesh').\n"
        "\n"
        "If the left operand evaluates to some SILO data object which has "
        "a structured type then the right operand should a the name of a "
        "field of that compound type. This operator will return the "
        "specified field of the SILO object.  For instance, if `qm' is "
        "of type `DBquadmesh' then the expression `qm.dims' returns the "
        "quadmesh dimension array.\n"
        "\n"
        "If the left operand evaluates to a compound datatype then the "
        "right operand should be the name of one of the fields of that "
        "datatype. This operator will return the datatype of the specified "
        "field from the left operand.\n"
        "\n"
        "If the left operand is an array datatype or a silo object of "
        "array datatype and the right operand is an integer then this "
        "operator returns the specified array element (using C-like "
        "zero-based indexing) Example: `mesh_3d.min_extents[0]'.\n"
        "\n"
        "If the left operand is an array datatype or a silo object of "
        "array datatype and the right operand is a range `MIN:MAX' then "
        "this operator returns a sub-array with elements MIN through "
        "MAX, inclusive. Example: `mesh_3d.min_extents[0:1]'\n"
        "\n"
        "If the left operand is an array datatype or a silo object of "
        "array datatype and the right operands are integers or ranges "
        "then one operand is applied to each array dimension in turn, "
        "automatically skipping over pointers. For instance, if a quadvar "
        "`qv' has a `vals' field of type `*[2]*[36000]float' then the "
        "expression `qv.vals[0,100:199]' will be an object consisting of the "
        "second 100 values of the first variable. (The expression "
        "`qv.vals[0][100:199]' would result in an errr because "
        "`qv.vals[0]' returns an object with the type `[1]*[36000]float' "
        "to which is then applied the `[100:199]', which is invalid because "
        "the array has only one element.\n"
        "\n"
        "If the left operand is a list then this operator is applied to "
        "each member of that list, returning a list of the results.");


   bif ("Pipe",         V_pipe,         HOLD,
        "Redirection operator (`|').",
        "Evaluates the left operand and redirects its output to the shell "
        "command specified by the right operand. The `|' operator and `Pipe' "
        "symbol are bound to the same function, but when invoked as the "
        "`Pipe' function a third argument is required. The third argument "
        "is a popen(3c) mode string.");
   
   bif ("Quote",        V_quote,        HOLD,
        "List quoting operator (`{}').",
        "This operator takes zero or more operands occurring between the "
        "curly brackes (optionally separated by commas) and returns a list "
        "of those unevaluated operands. If only one operand is given then "
        "that operand is returned unevaluated and not in a list. The `{}' "
        "operator and `Quote' symbol are bound to the same function.");

   bif ("Redirect",     V_redirect,     HOLD,
        "Redirection operators (`>', `>>').",
        "Evaluates the left operand and redirects its output to the file "
        "whose name is specified by the right operand.  The `>' and `>>' "
        "operators are bound to the same function as the `Redirect' symbol, "
        "but invoking the function as `Redirect' requires a third argument "
        "which is the fopen(3c) mode string.");

   bif(NULL, NULL, 0,
       "Range operator (`:').",
       "The range operator is used to construct a range object which "
       "represents all integers between the left and right operands, "
       "inclusive.");
}


/*-------------------------------------------------------------------------
 * Function:    bif
 *
 * Purpose:     Installs a builtin function in the symbol table.
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *              Robb Matzke, 2000-06-06
 *              Added the DESC and DOC arguments. DESC is an optional
 *              one-line description to be added to the function table of
 *              contents. DOC is a multi-line documentation string.
 *-------------------------------------------------------------------------
 */
static void
bif (const char *func_name, obj_t(*func_ptr)(int x, obj_t y[]), int flags,
     const char *desc, const char *doc) {

    obj_t       name = func_name?obj_new(C_SYM, func_name):NIL;
    obj_t       func = func_ptr?obj_new(C_BIF, func_ptr, flags):NIL;

    /* Bind the function to the symbol */
    if (name) sym_fbind(name, func);
   
    /* Function table of contents */
    if (name && desc) {
        HelpFuncToc[NHelpFuncToc].name = safe_strdup(func_name);
        HelpFuncToc[NHelpFuncToc].desc = safe_strdup(desc);
        NHelpFuncToc++;
    }

    /* Operator table of contents. Operators are added to the operator
     * table of contents if the description contains the string `operator'
    * and the operators are listed in quotes like `this'. */
    if (desc && strstr(desc, "operator")) {
        const char *s=desc, *first, *last;

        while ((first=strchr(s, '`')) && (last=strchr(++first, '\''))) {
            char opname[64];
            strcpy(opname, "op");
            strncpy(opname+2, first, last-first);
            opname[2+last-first] = '\0';
            if (opname[2]) {
                if (doc) {
                    obj_t sym = obj_new(C_SYM, opname);
                    obj_t docval = obj_new(C_STR, doc);
                    sym_dbind(sym, docval);
                    obj_dest(sym);
                }
                strcpy(opname, "\"op");
                strncpy(opname+3, first, last-first);
                strcpy(opname+(3+last-first), "\"");
                HelpOpToc[NHelpOpToc].name = safe_strdup(opname);
                HelpOpToc[NHelpOpToc].desc = safe_strdup(desc);
                NHelpOpToc++;
            }
            s = last+1;
        }
    }

    /* Bind the documentation string to the symbol */
    if (name && doc) {
        obj_t docval = obj_new(C_STR, doc);
        sym_dbind(name, docval);
    }
   
    if (name) obj_dest (name);
    /*dont destroy func or doc*/
}
   

/*-------------------------------------------------------------------------
 * Function:    parse_stmt
 *
 * Purpose:     Parses a statement which is a function name followed by
 *              zero or more arguments.
 *
 * Return:      Success:        Ptr to parse tree.
 *
 *              Failure:        NIL, input consumed through end of line.
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 11 Dec 1996
 *      If IMPLIED_PRINT is true then wrap the input in a call to the
 *      `print' function if it isn't already obviously a call to `print'.
 *
 *      Robb Matzke, 20 Jan 1997
 *      Turn off handling of SIGINT during parsing.
 *
 *      Robb Matzke, 7 Feb 1997
 *      If the first thing on the line is a symbol which has a built in
 *      function (BIF) as its f-value, and the BIF has the lex_special
 *      property, then we call lex_special() to prepare the next token.
 *
 *      Robb Matzke, 2000-06-28
 *      Signal handlers are registered with sigaction() since its behavior
 *      is more consistent.
 *
 *      Kathleen Bonnell, Thu Dec 9 09:36:44 PST 2010
 *      sigaction not defined on WIN32, ifdef it.
 *-------------------------------------------------------------------------
 */
obj_t
parse_stmt (lex_t *f, int implied_print) {

   char         *lexeme, buf[1024], *s, *fmode;
   int          tok, i;
   obj_t        head=NIL, opstack=NIL, b1=NIL, retval=NIL, tmp=NIL;

#ifndef _WIN32
   struct sigaction new_action, old_action;

   /* SIGINT should have the default action while we're parsing */
   new_action.sa_handler = SIG_DFL;
   sigemptyset(&new_action.sa_mask);
   new_action.sa_flags = SA_RESTART;
   sigaction(SIGINT, &new_action, &old_action);
#endif

   tok = lex_token (f, &lexeme, false);

   /*
    * At the end of the file, return `(exit)'.
    */
   if (TOK_EOF==tok) {
      lex_consume (f);
      if (f->f && isatty (fileno (f->f))) {
         printf ("exit\n");
         retval = obj_new (C_CONS,
                           obj_new (C_SYM, "exit"),
                           NIL);
         goto done;
      } else {
         retval = obj_new (C_SYM, "__END__");
         goto done;
      }
   }

   /*
    * For an empty line, eat the linefeed token and try again.
    */
   if (TOK_EOL==tok) {
      lex_consume (f);
      retval = parse_stmt (f, implied_print);
      goto done;
   }

   /*
    * A statement begins with a function name.  If the first token
    * is not a symbol then assume `print'.
    */
   if (implied_print && TOK_SYM==tok) {
      head = obj_new (C_SYM, lexeme);
      if ((tmp=sym_fboundp (head))) {
         tmp = obj_dest (tmp);
         lex_consume (f);
      } else {
         obj_dest (head);
         head = obj_new (C_SYM, "print");
      }
   } else if (implied_print) {
      head = obj_new (C_SYM, "print");
   } else {
      head = &ErrorCell ;       /*no function yet*/
   }

   /*
    * Some functions take a weird first argument that isn't really a
    * normal token.  Like `open' which wants the name of a file.  We
    * call lex_special() to try to get such a token if it appears
    * next.
    */
   if (head && &ErrorCell!=head && (tmp=sym_fboundp(head))) {
      if (bif_lex_special (tmp)) lex_special (f, false);
      tmp = obj_dest (tmp);
   }

   /*
    * Read the arguments...
    */
   while (&ErrorCell!=(b1=parse_expr(f, false))) {
      opstack = obj_new(C_CONS, b1, opstack);
   }

   /*
    * Construct a function call which is the HEAD applied to the
    * arguments on the operand stack.
    */
   b1 = F_reverse (opstack);
   opstack = obj_dest (opstack);

   if (&ErrorCell==head) {
      head = NIL;
      if (1==F_length(b1)) {
         retval = obj_copy (cons_head (b1), SHALLOW);
         b1 = obj_dest (b1);
      } else {
         retval = b1;
         b1 = NIL;
      }
   } else {
      retval = F_cons (head, b1);
      head = obj_dest (head);
      b1 = obj_dest (b1);
   }
      

   /*
    * A statement can end with `>' or `>>' followed by the name of
    * a file, or `|' followed by an unquoted shell command.  Leading
    * and trailing white space is stripped from the file or command.
    */
   tok = lex_token (f, &lexeme, false);
   if (TOK_RT==tok || TOK_RTRT==tok || TOK_PIPE==tok) {
      lex_consume (f);
      if (NULL==lex_gets (f, buf, sizeof(buf))) {
         out_errorn ("file name required after `%s' operator", lexeme);
         goto error;
      }
      lex_set (f, TOK_EOL, "\n");
      for (s=buf; isspace(*s); s++) /*void*/;
      for (i=strlen(s)-1; i>=0 && isspace(s[i]); --i) s[i] = '\0';
      if (!*s) {
         out_errorn ("file name required after `%s' operator", lexeme);
         goto error;
      }
      switch (tok) {
      case TOK_RT:
         lexeme = "Redirect";
         fmode = "w";
         break;
      case TOK_RTRT:
         lexeme = "Redirect";
         fmode = "a";
         break;
      case TOK_PIPE:
         lexeme = "Pipe";
         fmode = "w";
         break;
      default:
         abort();
      }
      retval = obj_new (C_CONS,
                        obj_new (C_SYM, lexeme),
                        obj_new (C_CONS,
                                 retval,
                                 obj_new (C_CONS,
                                          obj_new (C_STR, s),
                                          obj_new (C_CONS,
                                                    obj_new (C_STR, fmode),
                                                    NIL))));
   }

   /*
    * We should be at the end of a line.
    */
   tok = lex_token (f, &lexeme, false);
   if (TOK_EOL!=tok && TOK_EOF!=tok) {
      s = lex_gets (f, buf, sizeof(buf));
      if (s && strlen(s)>0 && '\n'==s[strlen(s)-1]) s[strlen(s)-1] = '\0';
      out_errorn ("syntax error before: %s%s", lexeme, s?s:"");
      lex_consume(f);
      goto error;
   } else {
      lex_consume(f);
   }

done:
#ifndef _WIN32
   sigaction(SIGINT, &old_action, NULL);
#endif
   return retval;

error:
   if (head) head = obj_dest (head);
   if (opstack) opstack = obj_dest (opstack);
   if (retval) retval = obj_dest (retval);
#ifndef _WIN32
   sigaction(SIGINT, &old_action, NULL);
#endif
   return NIL;
}


/*-------------------------------------------------------------------------
 * Function:    parse_term
 *
 * Purpose:     Parses a term which is a symbol, a string, or a number.
 *
 * Return:      Success:        Ptr to the term object or NIL
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *      Robb Matzke, 7 Feb 1997
 *      If the first thing after a parenthesis is a symbol which has a
 *      built in function (BIF) as its f-value, and the BIF has the
 *      lex_special property, then we call lex_special() to prepare the
 *      next token.
 *
 *      Robb Matzke, 26 Aug 1997
 *      The term `.' means current working directory.
 *-------------------------------------------------------------------------
 */
static obj_t
parse_term (lex_t *f, int skipnl) {

   char         *lexeme;
   obj_t        retval=&ErrorCell, opstack=NIL, tmp=NIL, fval=NIL;
   int          tok, nargs;

   switch ((tok=lex_token(f, &lexeme, skipnl))) {
   case TOK_DOT:
      retval = obj_new (C_SYM, lexeme);
      lex_consume (f);
      break;
      
   case TOK_SYM:
      if (!strcmp (lexeme, "nil")) {
         retval = NIL;
      } else {
         retval = obj_new (C_SYM, lexeme);
      }
      lex_consume (f);
      break;

   case TOK_NUM:
      retval = obj_new (C_NUM, lexeme);
      lex_consume (f);
      break;

   case TOK_STR:
      retval = obj_new (C_STR, lexeme);
      lex_consume (f);
      break;

   case TOK_LTPAREN:
      nargs = 0;
      lex_consume (f);
      while (TOK_RTPAREN!=(tok=lex_token(f, NULL, true)) && TOK_EOF!=tok) {

         /*
          * If the first token after the left paren is a symbol, and
          * the symbol has a BIF f-value, and the BIF has the lex_special
          * attribute, then call lex_special().
          */
         if (0==nargs++ && TOK_SYM==tok) {
            tmp = obj_new (C_SYM, f->lexeme);
            lex_consume (f);
            fval = sym_fboundp (tmp);
            if (bif_lex_special (fval)) lex_special (f, true);
            fval = obj_dest (fval);
         } else {
            tmp = parse_expr (f, true);
         }
         if (&ErrorCell==tmp) {
            opstack = obj_dest (opstack);
            goto done;
         }
         opstack = obj_new (C_CONS, tmp, opstack);
      }
      if (TOK_RTPAREN!=tok) {
         out_errorn ("right paren expected near end of input");
         opstack = obj_dest (opstack);
         goto done;
      }
      lex_consume (f);
      retval = F_reverse (opstack);
      opstack = obj_dest (opstack);
      break;

   case TOK_LTCURLY:
      /*
       * A list of items inside curly braces `{A B ... Z}' is just short for
       * `(Quote A B ... Z)' and `Quote' is like the LISP `quote' function in
       * that (Quote X) returns X without trying to evaluate it.  People tend
       * to use commas, so we accept commas between items.
       */
      lex_consume (f);
      while (TOK_RTCURLY!=(tok=lex_token(f, NULL, true)) && TOK_EOF!=tok) {
         tmp = parse_expr (f, true);
         if (&ErrorCell==tmp) {
            opstack = obj_dest (opstack);
            goto done;
         }
         opstack = obj_new (C_CONS, tmp, opstack);
         if (TOK_COMMA==lex_token(f, NULL, true)) lex_consume (f);
      }
      if (TOK_RTCURLY!=tok) {
         out_errorn ("right curly brace expected near end of input");
         opstack = obj_dest (opstack);
         goto done;
      }
      lex_consume (f);
      retval = F_reverse (opstack);
      opstack = obj_dest (opstack);
      retval = obj_new (C_CONS,
                        obj_new (C_SYM, "Quote"),
                        retval);
      break;
      
   }

done:
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    parse_range
 *
 * Purpose:     Tries to parse a range expression of the form `I1:I2'
 *              where I1 and I2 are integer constants.
 *
 * Return:      Success:        A range object.
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
parse_range (lex_t *f, int skipnl) {

   obj_t        lt=NIL, rt=NIL, retval=NIL;
   int          lo, hi;

   lt = parse_term (f, skipnl);
   if (&ErrorCell==lt) return &ErrorCell;

   if (TOK_COLON==lex_token (f, NULL, skipnl)) {
      lex_consume (f);
      rt = parse_term (f, skipnl);
      if (&ErrorCell==rt) {
         obj_dest (rt);
         return &ErrorCell;
      }

      /*
       * Both arguments must be integer constants.
       */
      if (!num_isint(lt)) {
         out_error ("Range: left limit is not an integer constant: ", lt);
         obj_dest (lt);
         obj_dest (rt);
         return &ErrorCell;
      }
      if (!num_isint(rt)) {
         out_error ("Range: right limit is not an integer constant: ", rt);
         obj_dest (lt);
         obj_dest (rt);
         return &ErrorCell;
      }

      /*
       * The constants must be in a reasonable order.
       */
      lo = num_int (lt);
      hi = num_int (rt);
      if (hi<lo) {
         out_errorn ("Range: inverted range %d:%d changed to %d:%d",
                     lo, hi, hi, lo);
         lo = num_int (rt);
         hi = num_int (lt);
      }

      /*
       * Create the range object.
       */
      lt = obj_dest (lt);
      rt = obj_dest (rt);
      retval = obj_new (C_RANGE, lo, hi);
   } else {
      retval = lt;
   }
   return retval;
}
      

/*-------------------------------------------------------------------------
 * Function:    parse_dot
 *
 * Purpose:     Tries to parse an expression followed by the dot operator
 *              followed by another expression.  If the dot is not present
 *              then just the left operand is returned.
 *
 * Return:      Success:        The expression.
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  5 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
parse_dot (lex_t *f, int skipnl) {

   obj_t        rt=NIL, retval=NIL;

   retval = parse_range (f, skipnl);
   if (&ErrorCell==retval) return &ErrorCell;

   while (TOK_DOT==lex_token(f, NULL, skipnl)) {
      lex_consume (f);
      rt = parse_range (f, skipnl);
      if (&ErrorCell==rt) {
         obj_dest (retval);
         return &ErrorCell;
      }
      retval = obj_new (C_CONS,
                        obj_new (C_SYM, "Dot"),
                        obj_new (C_CONS,
                                 retval,
                                 obj_new (C_CONS,
                                          rt,
                                          NIL)));
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    parse_subscripts
 *
 * Purpose:     Parses a subscripted expression.  The subscript is
 *              enclosed in `[' and `]' after the main expression.
 *
 * Return:      Success:        Ptr to the expression.
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Jan  3 1997
 *
 * Modifications:
 *
 *      Robb Matzke, 4 Feb 1997
 *      The contents of the `[]' can now be a comma-separated list
 *      of expressions.
 *
 *-------------------------------------------------------------------------
 */
static obj_t
parse_selection (lex_t *f, int skipnl) {

   obj_t        retval=NIL;             /*first argument, left of `['   */
   obj_t        tmp=NIL;                /*a subscript argument          */
   obj_t        operands=NIL;           /*operand list                  */
   int          septok;                 /*separator token               */

   retval = parse_dot (f, skipnl);
   if (&ErrorCell==retval) return &ErrorCell;

   /*
    * Zero or more array selectors.
    */
   while ('['==lex_token (f, NULL, skipnl)) {
      lex_consume (f);
      operands = obj_new (C_CONS, retval, NIL);
      retval = NIL;

      /*
       * One or more comma-separated expressions per selection.
       */
      for (;;) {
         tmp = parse_expr (f, skipnl);
         if (&ErrorCell==tmp) {
            obj_dest (retval);
            return &ErrorCell;
         }
         operands = obj_new (C_CONS, tmp, operands);    /*push*/
         septok = lex_token (f, NULL, skipnl);
         if (','==septok) {
            lex_consume (f);
         } else if (']'==septok) {
            lex_consume (f);
            break;
         } else {
            out_errorn ("expected ']'");
            obj_dest (operands);
            return &ErrorCell;
         }
      }

      /*
       * Put the operands in the correct order.
       */
      tmp = F_reverse (operands);
      obj_dest (operands);
      operands = tmp;
      tmp = NIL;
      
      /*
       * Add the function name `Dot' to the beginning of the
       * list.
       */
      retval = obj_new (C_CONS,
                        obj_new (C_SYM, "Dot"),
                        operands);
   }
   
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    parse_assignment
 *
 * Purpose:     Parses an assignment statement of the form
 *
 *                      LVALUE = RVALUE
 *
 * Return:      Success:        the resulting parse tree.
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              robb@maya.nuance.mdn.com
 *              Feb  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
parse_assignment (lex_t *f, int skipnl) {

   obj_t        rt=NIL, retval=NIL;

   retval = parse_selection (f, skipnl);
   if (&ErrorCell==retval) return &ErrorCell;

   if (TOK_EQ==lex_token (f, NULL, skipnl)) {
      lex_consume (f);
      rt = parse_selection (f, skipnl);
      if (&ErrorCell==rt) {
         obj_dest(retval);
         return &ErrorCell;
      }
      retval = obj_new (C_CONS,
                        obj_new (C_SYM, "Assign"),
                        obj_new (C_CONS,
                                 retval,
                                 obj_new (C_CONS,
                                          rt,
                                          NIL)));
   }
   return retval;
}


/*-------------------------------------------------------------------------
 * Function:    parse_expr
 *
 * Purpose:     Parses an expression.
 *
 * Return:      Success:        The expression.
 *
 *              Failure:        &ErrorCell
 *
 * Programmer:  Robb Matzke
 *              matzke@viper.llnl.gov
 *              Dec  4 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static obj_t
parse_expr (lex_t *f, int skipnl) {

   return parse_assignment (f, skipnl);
}
