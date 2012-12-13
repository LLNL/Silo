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
/*
 * PDBDIR.C - provides a directory capability for PDBLib
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */
#include "pdb.h"
#include <string.h>


/*-------------------------------------------------------------------------
 * Function:	lite_PD_cd
 *
 * Purpose:	Change the current working directory.  The directory
 *		may be specified by an absolute or relative path.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:41 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_cd (PDBfile *file, char *dirname) {

   char name[MAXLINE];
   syment *ep;

   lite_PD_err[0] = '\0';

   if (file == NULL) {
      sprintf(lite_PD_err, "ERROR: BAD FILE ID - PD_CD\n");
      return(FALSE);
   }
     
   if (dirname == NULL) {
      strcpy(name, "/");
   } else {
      strcpy(name, _lite_PD_fixname(file, dirname));
      if (name[strlen(name) - 1] != '/') strcat(name, "/");
   }

   ep = lite_PD_inquire_entry(file, name, FALSE, NULL);
   if (ep == NULL) {
      if (dirname == NULL) {
	 return(FALSE);
      } else {
	 if (strcmp(name, "/") != 0) {
	    name[strlen(name) - 1] = '\0';
	    ep = lite_PD_inquire_entry(file, name, FALSE, NULL);
	    strcat(name, "/");
	 }

	 if (ep == NULL) {
	    sprintf(lite_PD_err, "ERROR: DIRECTORY %s NOT FOUND - PD_CD\n",
		    dirname);
	    return(FALSE);
	 }
      }
   }

   if (strcmp(ep->type, "Directory") != 0) {
      sprintf(lite_PD_err, "ERROR: BAD DIRECTORY %s - PD_CD\n", dirname);
      return(FALSE);
   } else {
      if (file->current_prefix) SFREE(file->current_prefix);
      file->current_prefix = lite_SC_strsavef(name, "char*:PD_CD:name");
   }

   return(TRUE);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_ls
 *
 * Purpose:	Return a list of all variables and directories of the
 *		specified type in the specified directory.  If type is
 *		NULL, all types are returned.  If path is NULL, the root
 *		directory is searched.  Directories are terminated with
 *		a slash.
 *
 * Return:	Success:	Returns an array of pointers to strings.
 *				The array and the strings are allocated
 *				with score.  The vector of pointers is
 *				terminated with the null pointer.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:17 AM EST
 *
 * Modifications:
 *    Eric Brugger, Thu Dec 10 11:38:43 PST 1998
 *    I moved a free to be inside a loop to eliminate a memory leak.
 *
 *    Mark Miller, Wed Jun 11 16:42:09 PDT 2008
 *    Fixed valgrind error of src/dst overlap in strcpy
 *-------------------------------------------------------------------------
 */
char **
lite_PD_ls (PDBfile *file, char *path, char *type, int *num) {
   syment	*ep;
   char		**varlist, **outlist;
   char		*name;
   char		pattern[MAXLINE];
   int		nvars, i, has_dirs, head, pass;
     
   lite_PD_err[0] = '\0';

   *num = 0;

   if (file == NULL) {
      sprintf(lite_PD_err, "ERROR: BAD FILE ID - PD_LS\n");
      return(NULL);
   }

   if (num == NULL) {
      sprintf(lite_PD_err, "ERROR: LAST ARGUMENT NULL - PD_LS\n");
      return(NULL);
   }

   if (file->symtab->nelements == 0) return(NULL);
    
   /*
    * Determine if file contains directories and
    * build a pattern which names must match e.g., '/dir/abc*'
    */
   if (PD_has_directories(file)) {
      has_dirs = TRUE;
      if (path == NULL) {
	 if (strcmp(lite_PD_pwd(file), "/") == 0) strcpy(pattern, "/*");
	 else sprintf(pattern, "%s/*", lite_PD_pwd(file));
      } else {
	 strcpy(pattern, _lite_PD_fixname(file, path));
	 ep = lite_PD_inquire_entry(file, pattern, FALSE, NULL);
	 if ((ep != NULL) && (strcmp(ep->type, "Directory") == 0)) {
	    if (pattern[strlen(pattern) - 1] == '/') strcat(pattern, "*");
	    else strcat(pattern, "/*");
	 } else {
	    if (pattern[strlen(pattern) - 1] != '/') {
	       strcat(pattern, "/");
	       ep = lite_PD_inquire_entry(file, pattern, FALSE, NULL);
	       if ((ep != NULL) && (strcmp(ep->type, "Directory") == 0))
		  strcat(pattern, "*");
	       else
		  pattern[strlen(pattern) - 1] = '\0';
	    } else {
	       pattern[strlen(pattern) - 1] = '\0';
	       ep = lite_PD_inquire_entry(file, pattern, FALSE, NULL);
	       if ((ep != NULL) && (strcmp(ep->type, "Directory") == 0))
		  strcat(pattern, "/*");
	       else
		  strcat(pattern, "/");
	    }
	 }
      }
   } else {
      has_dirs = FALSE;
      if (path == NULL) strcpy(pattern, "*");
      else strcpy(pattern, path);
   }
     
   /*
    * Generate the list of matching names. Note that this returns items which
    * are in the requested directory AND items which are in sub-directories of
    * the requested directory. In other words, all names which BEGIN with the
    * requested pattern are returned.
    */
   nvars = 0;
   outlist = FMAKE_N(char *, file->symtab->nelements + 1, "PD_LS:outlist");
     
   /*
    * The second pass is in case variables were written to the file before
    * the first directory was created. Such variables lack an initial slash.
    */
   for (pass = 1; pass <= 2; pass++) {
      if (pass == 2) {
	 if (has_dirs && (strchr(pattern + 1, '/') == NULL)) {
            memmove(pattern, pattern+1, strlen(pattern+1)+1);
	 } else {
	    break;
	 }
      }

      varlist = lite_SC_hash_dump(file->symtab, pattern);
      if ((varlist == NULL) || (varlist[0] == NULL)) continue;
     
      /*
       * Save only those variables which are IN the requested directory
       * (not in sub-directories), and are of the requested type
       */
      for (i=0; (i<file->symtab->nelements) && (varlist[i]!=NULL); i++) {
	 /*
	  * The entry '/' (the root directory) is a special case. It
	  * is not a child of any directory, so should be ignored.
	  */
	 if (strcmp("/", varlist[i]) == 0) continue;
          
	 /*
	  * Check to see if type of this variable matches request.
	  */
	 if (type != NULL) {
	    ep = lite_PD_inquire_entry(file, varlist[i], FALSE, NULL);
	    if (strcmp(ep->type, type) != 0) continue;
	 }

	 /*
	  * If here, then variable is of right type. If this file has
	  * directories, check for any more slashes (/'s) in the
	  * name. If any are found, this is not a leaf element. NOTE:
	  * if directories are not used, slashes are valid charcters
	  * in file names.
	  */
	 if (has_dirs) {
	    if (pattern[0] != '/') head = 0;
	    else head = strlen(pattern) - strlen(strrchr(pattern, '/')) + 1;
	    name = &(varlist[i])[head];
	    if ((strlen(name) == 0) ||
		((pass == 2) && (name[0] == '/')) ||
		((strchr(name, '/') != NULL) &&
		 (strchr(name, '/') != ((name + strlen(name) - 1)))))
	       continue;
	 } else {
	    name = varlist[i];
	 }
          
	 /*
	  * Variable is of right type and is a leaf in the requested
	  * directory.
	  */
	 outlist[nvars++] = name;
      }
      SFREE(varlist);
   }
     
   /*
    * Store a null string to terminate list (just a precaution)
    */
   outlist[nvars] = NULL;

   if (has_dirs) lite_SC_string_sort(outlist, nvars);
   *num = nvars;
     
   return(outlist);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_pwd
 *
 * Purpose:	Returns the current working directory.
 *
 * Return:	Success:	A ptr to a statically allocated buffer
 *				which contains the name of the current
 *				directory.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:31 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_PD_pwd(PDBfile *file) {

   static char		cwd[MAXLINE];

   lite_PD_err[0] = '\0';

   if (file == NULL) {
      sprintf(lite_PD_err, "ERROR: BAD FILE ID - PF_PWD\n");
      return(NULL);
   }

   if ((file->current_prefix == NULL) ||
       (strcmp(file->current_prefix, "/") == 0)) {
      strcpy(cwd, "/");
   } else {
      int cwdlen;
      strcpy(cwd, file->current_prefix);
      cwdlen = strlen(cwd);
      cwd[cwdlen?cwdlen-1:0] = '\0';
   }
   return(cwd);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_PD_fixname
 *
 * Purpose:	Make full pathname from current working directory
 *		and the given pathname (absolute or relative)
 *
 * Return:	Success:	Ptr to a static character buffer which
 *				holds the name.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996  4:40 PM EST
 *
 * Modifications:
 *  Sean Ahern, Thu Jul  2 11:01:32 PDT 1998
 *  Fixed some indexing problems on strings.
 *
 *-------------------------------------------------------------------------
 */
char *
_lite_PD_fixname (PDBfile *file, char *inname) {

   static char	outname[MAXLINE];
   char 	*node;
   char 	tmpstr[MAXLINE];

   if ((file == NULL) || (inname == NULL)) return(NULL);

   outname[0] = '\0';

   if (!PD_has_directories(file)) {
      /*
       * If no directories, just copy verbatim.
       */
      strcpy(outname, inname);
   } else {
      /*
       * Break path into slash-separated tokens.
       * Process each node individually.
       */
      if (inname[0] != '/') strcpy(outname, lite_PD_pwd(file));
      strcpy(tmpstr, inname);
      node = (char *) strtok(tmpstr, "/");
          
      while (node != NULL) {
	 if (strcmp(".",  node) == 0) {
	    /*void*/
	 } else if (strcmp("..", node) == 0) {
	    /*
	     *	Go up one level, unless already at top.
	     */
	    if (strcmp("/", outname) != 0) {
	       char  *s;
               int onlen = strlen(outname);
	       if (outname[onlen?onlen-1:0] == '/') {
		  outname[onlen?onlen-1:0] = '\0';
	       }
	       s = strrchr(outname, '/');
	       if (s != NULL) s[0] = '\0';
	    }
	 } else {
	    /*
	     * Append to end of current path.
	     */
            int onlen = strlen(outname);
	    if ((onlen == 0) || (outname[onlen-1] != '/'))
                strcat(outname, "/");
	    strcat(outname, node);
	 }
	 node = (char *) strtok(NULL, "/");
      }

      if ((strlen(inname) > 0) &&
          (inname[strlen(inname) - 1] == '/') &&
          ((strlen(outname) == 0) || 
	   (outname[strlen(outname) - 1] != '/')))
	 strcat(outname, "/");
   }

   if (outname[0] == '\0') strcpy(outname, "/");
   return(outname);
}



/*-------------------------------------------------------------------------
 * Function:	lite_PD_mkdir
 *
 * Purpose:	Create a directory.  The directory may be specified by an
 *		absolute or relative path.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
lite_PD_mkdir (PDBfile *file, char *dirname) {

   int dir;
   char name[MAXLINE], head[MAXLINE];
   char *s;
   static int dir_num = 0;
     
   lite_PD_err[0] = '\0';

   if (file == NULL) {
      sprintf(lite_PD_err, "ERROR: BAD FILE ID - PD_MKDIR\n");
      return(FALSE);
   }

   if (dirname == NULL) {
      sprintf(lite_PD_err, "ERROR: DIRECTORY NAME NULL - PD_MKDIR\n");
      return(FALSE);
   }
     
   /*
    * Define type "Directory", if it hasn't been already.
    */
   if (!PD_has_directories(file)) {
      if ((lite_PD_defncv(file, "Directory", 1, 0)) == NULL) return FALSE;
          
      /*
       * Write out the root directory.
       */
      dir  = dir_num;
      if (!lite_PD_write(file, "/", "Directory", &dir)) return(FALSE);
      dir_num++;
   }

   /*
    * Build an absolute pathname.
    */
   strcpy(name, _lite_PD_fixname(file, dirname));
   if (name[strlen(name) - 1] != '/') strcat(name, "/");

   /*
    * Make sure this directory hasn't already been created.
    */
   if (lite_PD_inquire_entry(file, name, FALSE, NULL) != NULL) {
      sprintf(lite_PD_err, "ERROR: DIRECTORY %s ALREADY EXISTS - PD_MKDIR\n",
	      name);
      return(FALSE);
   }

   /*
    * Make sure the next higher level directory already exists.
    */
   strcpy(head, name);
   head[strlen(head) - 1] = '\0';
   s = strrchr(head, '/');
   if (s != NULL) {
      s[1] = '\0';
      if (lite_PD_inquire_entry(file, head, FALSE, NULL) == NULL) {
         int hlen = strlen(head);
	 head[hlen?hlen-1:0] = '\0';
	 sprintf(lite_PD_err, "ERROR: DIRECTORY %s DOES NOT EXIST - "
		 "PD_MKDIR\n", head);
	 return(FALSE);
      }
   }

   /*
    * Write the directory variable.
    */
   dir = dir_num;
   if (!lite_PD_write(file, name, "Directory", &dir)) return(FALSE);
   dir_num++;
     
   return(TRUE);
}
#endif /* PDB_WRITE */
