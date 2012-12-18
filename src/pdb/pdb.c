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
 * PDB.C - Portable Data Base Library
 *       - a collection of file manipulation routines with the following
 *       - aims:
 *       -      (1) Portable, therefore written in C
 *       -      (2) Machine Independent, carries information about objects
 *       -          in the file so that the implementation can extract
 *       -          the data even if on a wholely different machine type
 *       -      (3) Simplicity, for ease of implementation and so that
 *       -          linkable modules in another language can use these
 *       -          routines
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */

#include "config.h" /* For a possible redefinition of setjmp/longjmp. */
#if HAVE_STDARG_H
#include <stdarg.h>
#endif
#if !defined(_WIN32)
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#else
#include <silo_win32_compatibility.h>
#endif
#include <assert.h>
#include "pdb.h"


/*
 * PDB header token to uniquely identify as PDB file.
 */
#define HeadTok              "!<<PDB:II>>!"
#define OldHeadTok           "!<><PDB><>!"
#define PDB_ATTRIBUTE_TABLE  "!pdb_att_tab!"

#define PAD_SIZE ((size_t) 128)

jmp_buf		_lite_PD_open_err ;
jmp_buf		_lite_PD_print_err;
jmp_buf		_lite_PD_read_err ;
jmp_buf		_lite_PD_trace_err ;
jmp_buf		_lite_PD_close_err ;
jmp_buf		_lite_PD_write_err ;
jmp_buf		_lite_PD_create_err ;
char		lite_PD_err[MAXLINE];
int		lite_PD_buffer_size = -1;
ReaderFuncType	lite_pdb_rd_hook = NULL;
WriterFuncType	lite_pdb_wr_hook = NULL;
char           *lite_PD_DEF_CREATM = "w";

#ifdef PDB_WRITE
static int	_append_flag = FALSE ;
data_standard	*lite_REQ_STANDARD = NULL;
data_alignment	*lite_REQ_ALIGNMENT = NULL;
static syment *	_PD_write (PDBfile*,char*,char*,char*,lite_SC_byte*,dimdes*,int);
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_close
 *
 * Purpose:	Close a PDB file.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from the PACT PDB library
 *		Mar  4, 1996 10:46 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_close(PDBfile *file) {
   FILE		*fp;
   int		ret = TRUE ;

   switch (setjmp(_lite_PD_close_err)) {
   case ABORT:
      return(FALSE);
   case ERR_FREE:
      return(TRUE);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   fp  = file->stream;

   /*
    * Position the file pointer at the greater of the current position and
    * the location of the chart.
    */
#ifdef PDB_WRITE
   if (PD_CREATE==file->mode || PD_APPEND==file->mode) {
      ret = lite_PD_flush (file) ;
   }
#endif
   if (io_close(fp) != 0) {
      lite_PD_error("CAN'T CLOSE FILE - PD_CLOSE", PD_CLOSE);
   }

   /*
    * Free the space
    */
   _lite_PD_rl_pdb(file);
   return ret ;
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_defncv
 *
 * Purpose:	Define a new primitive type that will be format converted.
 *		Do it in both charts.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:42 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
defstr *
lite_PD_defncv (PDBfile *file, char *name, long bytespitem, int align) {

   defstr *dp;

   dp = _lite_PD_mk_defstr(name, NULL, bytespitem, align, -1, FALSE,
			   NULL, NULL);
   if (dp == NULL) {
      sprintf(lite_PD_err, "ERROR: DEFINITION FAILED - PD_DEFNCV\n");
   } else {
      _lite_PD_d_install(name, dp, file->chart);

      /*
       * Install an independent copy in the host chart - garbage collection!
       */
      dp = _lite_PD_mk_defstr(name, NULL, bytespitem, align, -1,
			      -1, NULL, NULL);
      _lite_PD_d_install(name, dp, file->host_chart);
   }

   return(dp);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_error
 *
 * Purpose:	Signal an error.  
 *
 * Return:	__NORETURN void
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:44 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
lite_PD_error(char *s, int n) {

   if (lite_PD_err[0] == '\0') sprintf(lite_PD_err, "ERROR: %s\n", s);
   switch (n) {
   case PD_OPEN:
      longjmp(_lite_PD_open_err, ABORT);
   case PD_TRACE:
      longjmp(_lite_PD_trace_err, ABORT);   
   case PD_CLOSE:
      longjmp(_lite_PD_close_err, ABORT);   
   case PD_READ:
      longjmp(_lite_PD_read_err, ABORT);   
   case PD_PRINT:
      longjmp(_lite_PD_print_err, ABORT);
   case PD_WRITE:
      longjmp(_lite_PD_write_err, ABORT);
   case PD_CREATE:
      longjmp(_lite_PD_create_err, ABORT);
   default:
      abort() ;
   }
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_open
 *
 * Purpose:	Open an existing PDB file, extract the symbol table and
 *		structure chart.
 *
 * Return:	Success:	Ptr to the PDB file structure
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from the PACT PDB library
 *		Mar  4, 1996 10:26 AM EST
 *
 * Modifications:
 *
 * 	Robb Matzke, 4 Mar 1996
 *	Fixed indentation.  Files can only be opened with mode `r'.
 *
 * 	Robb Matzke, 17 Apr 1996
 *	Added write capability back into the function, but it is protected
 *	with #ifdef PDB_WRITE.
 *
 *      Mark C. Miller, Fri Apr 13 22:37:56 PDT 2012
 *      Changed mode string checks to strchr to accomodate wider variety
 *      of mode characters for new hash table size and open modes.
 *
 *      Mark C. Miller, Thu Jun 14 13:25:02 PDT 2012
 *      Remove call to io_close in ABORT case. The file pointer may not
 *      have been properly initialized.
 *-------------------------------------------------------------------------
 */
PDBfile *
lite_PD_open (char *name, char *mode) {

   char		str[MAXLINE], *token;
   PDBfile 	*file=NULL;
   static FILE 	*fp;
   syment 	*ep;

#ifdef PDB_WRITE
   /*
    * If opened in write mode use PD_CREATE instead.
    */
   if (strchr(mode,'w')) return lite_PD_create (name);
#else
   assert (!strchr(mode,'r')) ;
#endif

   switch (setjmp(_lite_PD_open_err)) {
   case ABORT:
      if (fp) io_close(fp);
      return(NULL);
   case ERR_FREE:
      return(file);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * Open the file
    */
   strcpy(str, name);

#ifdef PDB_WRITE
   fp = io_open(str, BINARY_MODE_RPLUS);
   if (fp == NULL) {
      if (strchr(mode,'r')) {
#endif
	 fp = io_open(str, BINARY_MODE_R);
	 if (fp == NULL) {
	    lite_PD_error("CAN'T OPEN FILE IN READ-ONLY MODE - PD_OPEN",
			  PD_OPEN);
	 }
#ifdef PDB_WRITE
      } else if (strchr(mode,'a')) {
	 return lite_PD_create (name);
      } else {
	 lite_PD_error("CAN'T OPEN FILE - PD_OPEN", PD_OPEN);
      }
   }
#endif

   if (lite_PD_buffer_size != -1) {
      if (io_setvbuf(fp, NULL, _IOFBF, (size_t) lite_PD_buffer_size)) {
	 lite_PD_error("CAN'T SET FILE BUFFER - PD_OPEN", PD_OPEN);
      }
   }

   file = _lite_PD_mk_pdb(str, mode);
   if (file == NULL) {
      lite_PD_error("CAN'T ALLOCATE PDBFILE - PD_OPEN", PD_OPEN);
   }
   file->stream = fp;
#ifdef PDB_WRITE
   if (strchr(mode,'a')) file->mode = PD_APPEND;
   else file->mode = PD_OPEN;
#else
   file->mode = PD_OPEN ;
#endif

   /*
    * Attempt to read an ASCII header.
    */
   if (io_seek(fp, 0L, SEEK_SET)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("FSEEK FAILED TO FIND ORIGIN - PD_OPEN", PD_OPEN);
   }
   if (_lite_PD_rfgets(str, MAXLINE, fp) == NULL) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T READ THE FILE HEADER - PD_OPEN", PD_OPEN);
   }

   /*
    * The first token should be the identifying header token.
    */
   token = strtok(str, " ");
   if (token == NULL) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("FILE HEADER NOT ASCII - PD_OPEN", PD_OPEN);
   }


   if (strcmp(token, HeadTok) == 0) {
      /*
       * This is a PDB_SYSTEM_VERSION 2 or later file.
       * Read the primitive data type formats which set the standard.
       */
      if (!_lite_PD_rd_format(file)) {
         _lite_PD_rl_pdb(file); 
	 lite_PD_error("FAILED TO READ FORMATS - PD_OPEN", PD_OPEN);
      }
      
   } else if (strcmp(token, OldHeadTok) == 0) {
      /*
       * This is a pre-PDB_SYSTEM_VERSION 2 style file. The second token
       * is the machine type that wrote the file.  Set the file->std for
       * machine type for PD_open the file->std is always the PDBfile standard.
       * Alignment issues are not properly handled before PDB_SYSTEM_VERSION 3
       * but do the best that we can.
       */
      token = strtok(NULL, " ");
      if (token == NULL)
      {
         _lite_PD_rl_pdb(file); 
         lite_PD_error("INCOMPLETE HEADER - PD_OPEN", PD_OPEN);
      }
      switch (atoi(token)) {
      case IEEE_32_64:
	 file->std   = _lite_PD_copy_standard(&lite_IEEEA_STD);
	 file->align = _lite_PD_copy_alignment(&lite_M68000_ALIGNMENT);
	 break;
      case IEEE_32_96:
	 file->std   = _lite_PD_copy_standard(&lite_IEEEB_STD);
	 file->align = _lite_PD_copy_alignment(&lite_M68000_ALIGNMENT);
	 break;
      case INTEL_X86:
	 file->std   = _lite_PD_copy_standard(&lite_INTELA_STD);
	 file->align = _lite_PD_copy_alignment(&lite_INTELA_ALIGNMENT);
	 break;
      case CRAY_64:
	 file->std   = _lite_PD_copy_standard(&lite_CRAY_STD);
	 file->align = _lite_PD_copy_alignment(&lite_UNICOS_ALIGNMENT);
	 break;
      case VAX_11:
	 file->std   = _lite_PD_copy_standard(&lite_VAX_STD);
	 file->align = _lite_PD_copy_alignment(&lite_DEF_ALIGNMENT);
	 break;
      default:
	 file->std   = _lite_PD_copy_standard(&lite_DEF_STD);
	 file->align = _lite_PD_copy_alignment(&lite_DEF_ALIGNMENT);
	 break;
      }

      /*
       * To correctly handle the situation in which many PDBfiles are open
       * at the same time always try to latch on to the file->host_std.
       * Alignment issues are not properly handled before PDB_SYSTEM_VERSION 3
       * but do the best that we can
       */
      if (_lite_PD_compare_std(file->host_std, file->std,
			       file->host_align, file->align)) {
	 _lite_PD_rl_standard(file->std);
	 file->std   = _lite_PD_copy_standard(file->host_std);
	 _lite_PD_rl_alignment(file->align);
	 file->align = _lite_PD_copy_alignment(file->host_align);
      }
   } else {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("BAD FILE HEADER - PD_OPEN", PD_OPEN);
   }

   /*
    * Record the current file position as the location of the symbol table
    * address and sequentially the chart address.
    */
   file->headaddr = io_tell(fp);
   if (file->headaddr == -1L) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T FIND HEADER ADDRESS - PD_OPEN", PD_OPEN);
   }

   /*
    * Read the address of the symbol table and structure chart.
    */
   if (_lite_PD_rfgets(str, MAXLINE, fp) == NULL) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T READ SYMBOL TABLE ADDRESS - PD_OPEN", PD_OPEN);
   }

   token = strtok(str, "\001");
   if (token == NULL) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("BAD STRUCTURE CHART ADDRESS - PD_OPEN", PD_OPEN);
   }
   file->chrtaddr = atol(token);

   token = strtok(NULL, "\001");
   if (token == NULL) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("BAD SYMBOL TABLE ADDRESS - PD_OPEN", PD_OPEN);
   }
   file->symtaddr = atol(token);

   /*
    * Read the symbol table first so that the file pointer is positioned
    * to the "extra" information, then read the "extra's" to get the
    * alignment data, and finish with the structure chart which needs
    * the alignment data
    */

   /*
    * Read the symbol table.
    */
   if (io_seek(fp, file->symtaddr, SEEK_SET)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("FSEEK FAILED SYMBOL TABLE - PD_OPEN", PD_OPEN);
   }
   if (!_lite_PD_rd_symt(file)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T READ SYMBOL TABLE - PD_OPEN", PD_OPEN);
   }

   /*
    * Read the miscellaneous data.
    */
   if (!_lite_PD_rd_extras(file)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T READ MISCELLANEOUS DATA - PD_OPEN", PD_OPEN);
   }

   /*
    * Initialize the pdb system defs and structure chart.
    */
   _lite_PD_init_chrt(file);

   /*
    * Read the structure chart.
    */
   if (io_seek(fp, file->chrtaddr, SEEK_SET)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("FSEEK FAILED STRUCTURE CHART - PD_OPEN", PD_OPEN);
   }
   if (!_lite_PD_rd_chrt(file)) {
      _lite_PD_rl_pdb(file); 
      lite_PD_error("CAN'T READ STRUCTURE CHART - PD_OPEN", PD_OPEN);
   }

   ep = lite_PD_inquire_entry(file, PDB_ATTRIBUTE_TABLE, TRUE, NULL);
   if (ep != NULL) {
      if (!lite_PD_read(file, PDB_ATTRIBUTE_TABLE, &file->attrtab)) {
	 lite_PD_close(file);
	 lite_PD_error("FAILED TO READ ATTRIBUTE TABLE - PD_OPEN", PD_OPEN);
      }
      _lite_PD_convert_attrtab(file);
      file->chrtaddr = PD_entry_address(ep);
      _lite_PD_rl_syment(ep);
      lite_SC_hash_rem(_lite_PD_fixname(file, PDB_ATTRIBUTE_TABLE),
		       file->symtab);
   } else {
      file->attrtab = NULL;
   }

   /*
    * Position the file pointer to the location of the structure chart.
    */
   if (io_seek(fp, file->chrtaddr, SEEK_SET)) {
      lite_PD_close(file);
      lite_PD_error("FSEEK FAILED CHART - PD_OPEN", PD_OPEN);
   }

   return(file);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_read
 *
 * Purpose:	Read an entry from the PDB file pointed to by the
 *		symbol table into the location pointer to by VR.
 *
 * Note:	VR must be a pointer to an object with the type
 *		given by TYPE (PDBLib will allocate space if necessary)!
 *
 * Return:	Success:	The number of items successfully read.
 *
 *		Failure:	0
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:54 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_read (PDBfile *file, char *name, lite_SC_byte *vr) {

   return lite_PD_read_as (file, name, NULL, vr);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_read_alt
 *
 * Purpose:	Read part of an entry from the PDB file pointed to by
 *		the symbol table into the location pointed to by VR.  IND
 *		contains one triplen of long ints per variable dimension
 *		specifying start, stop, and step for the index.
 *
 * Note:	VR must be a pointer to an object with the type given
 *		by TYPE (PDBLib will allocate space if necessary)!
 *
 * Return:	Success:	Number of items successfully read.
 *
 *		Failure:	0
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:47 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_read_alt (PDBfile *file, char *name, lite_SC_byte *vr, long *ind) {

   return lite_PD_read_as_alt (file, name, NULL, vr, ind) ;
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_read_as
 *
 * Purpose:	Read an entry from the PDB file pointed to by the
 *		symbol table into the location pointed to by VR.  Convert
 *		to type TYPE regardless of symbol entry type.
 *
 * Note:	VR must be a pointer to an object with the type given
 *		by TYPE (PDBLib will allocate space if necessary)!
 *
 * Return:	Success:	Number of items successfully read.
 *
 *		Failure:	0
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:55 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_read_as (PDBfile *file, char *name, char *type, lite_SC_byte *vr) {

   int		err;
   syment	*ep;
   char 	msg[MAXLINE], fullpath[MAXLINE];

   switch (setjmp(_lite_PD_read_err)) {
   case ABORT:
      return(FALSE);
   case ERR_FREE:
      return(TRUE);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * Find the effective symbol table entry for the named item.
    */
   ep = _lite_PD_effective_ep(file, name, TRUE, fullpath);
   if (ep == NULL) {
      if (snprintf(msg, sizeof(msg), "UNREADABLE OR MISSING ENTRY \"%s\" - PD_READ_AS",fullpath) >= sizeof(msg))
          msg[sizeof(msg)-1] = '\0';
      lite_PD_error(msg, PD_READ);
   }

   if (type == NULL) type = PD_entry_type(ep);

   err = _lite_PD_hyper_read(file, fullpath, type, ep, vr);
   _lite_PD_rl_syment_d(ep);

   return(err);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_read_as_alt
 *
 * Purpose:	Read part of an entry from the PDB file pointed to by the
 *		symbol table into the location pointed to by VR.  IND contains
 *		one triplet of long ints per variable dimension specifying
 *		start, stop, and step for the index.
 *
 * Note:	The entry must be an array (either a static array or
 *		a pointer)
 *
 * Note:	VR must be a pointer to an object with the type of
 *		the object associated with NAME (PDBLib will allocate space
 *		if necessary)!
 *
 * Return:	Success:	Number of items successfully read.
 *
 *		Failure:	0
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  4, 1996 11:49 AM EST
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_PD_read_as_alt (PDBfile *file, char *name, char *type, lite_SC_byte *vr,
		     long *ind) {

   char		fullpath[MAXLINE];
   dimdes	*pd, *dims;
   syment	*ep;
   int		nd;

   switch (setjmp(_lite_PD_read_err)) {
   case ABORT:
      return(FALSE);
   case ERR_FREE:
      return(TRUE);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * Look up the variable name and return FALSE if it is not there.
    */
   ep = _lite_PD_effective_ep(file, name, TRUE, fullpath);
   if (ep == NULL)
      lite_PD_error("ENTRY NOT IN SYMBOL TABLE - PD_READ_AS_ALT", PD_READ);

   dims = PD_entry_dimensions(ep);
   for (nd = 0, pd = dims; pd != NULL; pd = pd->next, nd++) /*void*/ ;

   return _lite_PD_indexed_read_as (file, fullpath, type, vr, nd, ind, ep);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_typedef
 *
 * Purpose:	Define an alias for a type which exists in the host
 *		chart.  The indented use is to provide a correspondence
 *		between a type that has been defined to PDBLib (ONAME) and
 *		a typedef'd type in programs (TNAME).  Can be used in
 *		conjunction with PD_defix and PD_defloat to have a primitive
 *		type known to both charts.
 *
 * Return:	Success:	ptr to the original type's defstr
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT PDB
 *		Mar  6, 1996 11:47 AM EST
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:36:49 PST 1998
 *    I added calls to lite_SC_mark to bump memory reference counts as
 *    appropriate.
 *
 *-------------------------------------------------------------------------
 */
defstr *
lite_PD_typedef (PDBfile *file, char *oname, char *tname) {

   defstr *dp;

   dp = PD_inquire_host_type(file, oname);
   if (dp == NULL) {
      sprintf(lite_PD_err, "ERROR: HOST TYPE %s UNKNOWN - PD_TYPEDEF\n",
	      oname);
   } else {
      if (PD_inquire_host_type(file, tname) == NULL) {
	 _lite_PD_d_install(tname, dp, file->host_chart);
         lite_SC_mark(dp, 1);
         lite_SC_mark(dp->order, 1);
      }
   }

   dp = PD_inquire_type(file, oname);
   if (dp == NULL) {
      sprintf(lite_PD_err, "ERROR: FILE TYPE %s UNKNOWN - PD_TYPEDEF\n",
	      oname);
   } else {
      if (PD_inquire_type(file, tname) == NULL) {
	 _lite_PD_d_install(tname, dp, file->chart);
         lite_SC_mark(dp, 1);
         lite_SC_mark(dp->order, 1);
      }
   }

   return(dp);
}


/*-------------------------------------------------------------------------
 * Function:	lite_PD_write
 *
 * Purpose:	Write NUMBER VAR's ofj type TYPE to the PDB file, FILE.
 *		Make an entry in the file's symbol table.  VR must be
 *		a pointer to an object with the type given by TYPE!
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
lite_PD_write (PDBfile *file, char *name, char *type, lite_SC_byte *vr) {

   return lite_PD_write_as (file, name, type, type, vr) ;
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_write_as
 *
 * Purpose:	Write NUMBER VAR's of type INTYPE to the pdb file FILE as
 *		type OUTTYPE.  Make an entry in the file's symbol table.  VR
 *		must be a pointer to an object with the type given by TYPE!
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
lite_PD_write_as (PDBfile *file, char *name, char *intype, char *outtype,
		  lite_SC_byte *vr) {
   
   syment *ep;
   dimdes *dims;
   char *lname, fullpath[MAXLINE];

   strcpy(fullpath, _lite_PD_fixname(file, name));
   lname = lite_SC_firsttok(fullpath, ".");

   dims = _lite_PD_ex_dims(lname, file->default_offset, FALSE);
   ep   = _PD_write(file, name, intype, outtype, vr, dims, _append_flag);
   if (ep != NULL) {
      _lite_PD_rl_syment_d(ep);
      return(TRUE);
   } else {
      return(FALSE);
   }
}
#endif /* PDB_WRITE */
   

/*-------------------------------------------------------------------------
 * Function:	lite_PD_write_alt
 *
 * Purpose:	Write an entry of type TYPE to the PDB file, FILE.  Make
 *		an entry in the file's symbol table.  The entry is named
 *		by NAME has ND dimensions and IND contains the min and max
 *		(pairwise) of each dimensions range.  Return the syment
 * 		if successful and NULL otherwise.  VR must be a pointer
 *		to an object with the type given by TYPE.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	
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
lite_PD_write_alt (PDBfile *file, char *name, char *type, lite_SC_byte *vr, int nd,
		   long *ind) {

   return lite_PD_write_as_alt (file, name, type, type, vr, nd, ind) ;
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_write_as_alt
 *
 * Purpose:	Write an entry of type INTYPE to the PDB file, FILE as
 *		type OUTTYPE.  Make an entry in the file's symbol table.
 *		The entry has name, NAME, ND dimensions, and the ranges
 *		of the dimensions are given (min,max) pairwise in IND.
 *		VR must be a pointer to an object with the type given
 *		by TYPE.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:36:49 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *    Mark C. Miller, Mon Mar 13 10:51:44 PST 2006
 *    Added code to release dimensions just prior to returning false to
 *    fix a memory leak
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
lite_PD_write_as_alt (PDBfile *file, char *name, char *intype, char *outtype,
		      lite_SC_byte *vr, int nd, long *ind) {
   int i;
   long start, stop, step, leng;
   char expr[MAXLINE], index[MAXLINE], hname[MAXLINE];
   dimdes *dims, *next, *prev;
   syment *ep;

   prev = NULL;
   dims = NULL;

   strcpy(index, "(");

   for (i = 0; i < nd; i++) {
      start = ind[0];
      stop  = ind[1];
      step  = ind[2];
      ind += 3;

      sprintf(expr, "%ld:%ld:%ld,", start, stop, step);
      strcat(index, expr);

      leng = stop - start + 1L;
      next = _lite_PD_mk_dimensions(start, leng);
      if (dims == NULL) {
	 dims = next;
      } else {
	 prev->next = next;
      }

      prev = next;
   }

   if (strlen(index) > 1) {
      index[strlen(index)-1] = ')';
      sprintf(hname, "%s%s", name, index);
   } else {
      strcpy(hname, name);
   }

   ep  = _PD_write(file, hname, intype, outtype, vr, dims, _append_flag);

   if (ep != NULL) {
      _lite_PD_rl_syment_d(ep);
      return(TRUE);
   } else {
      _lite_PD_rl_dimensions(dims);
      return(FALSE);
   }
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	_PD_write
 *
 * Purpose:	Primitive write to PDBfile which is used by PD_WRITE and
 *		PD_WRITE_AS.  VR must be a pointer to an object with the
 *		type given by TYPE!
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *   Mark C. Miller, Mon Mar 13 10:51:44 PST 2006
 *   Added code to release dimensions in case we're overwriting
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
static syment *
_PD_write (PDBfile *file, char *name, char *intype, char *outtype,
	   lite_SC_byte *vr, dimdes *dims, int appnd) {

   int reset;
   syment *ep;
   long number, addr;
   char bf[MAXLINE], fullpath[MAXLINE], *lname;

   _append_flag = FALSE;

   ep = NULL;

   switch (setjmp(_lite_PD_write_err)) {
   case ABORT    : return(NULL);
   case ERR_FREE : return(ep);
   default       : memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   if (file->mode == PD_OPEN) {
      lite_PD_error("FILE OPENED IN READ-ONLY MODE - _PD_WRITE", PD_WRITE);
   }

   strcpy(fullpath, _lite_PD_fixname(file, name));

   /*
    * Append a new block to an existing entry if TRUE.
    */
   if (appnd) {
      strcpy(bf, fullpath);

      /*
       * Do this so that things such as a[20:20].b work properly
       * NOTE: this also implies that a[20:20].b.c works while
       *       a.b[20:20].c doesn't
       * for now this defines the semantics of append (10/6/93)
       */
      lname = lite_SC_firsttok(bf, ".()[]");

      ep = lite_PD_inquire_entry(file, lname, FALSE, NULL);
      if (ep == NULL) {
	 lite_PD_error("CAN'T APPEND TO NON-EXISTING ENTRY - _PD_WRITE",
		       PD_WRITE);
      }
      _lite_PD_adj_dimensions(file, fullpath, ep);

      /*
       * Extend the syment.
       */
      _lite_PD_add_block(file, ep, dims);
   }

   addr = file->chrtaddr;
   ep   = _lite_PD_effective_ep(file, fullpath, FALSE, NULL);

   if (ep != NULL) {
      /*
       * If the variable already exists use the existing file info.
       */
      addr   = PD_entry_address(ep);
      _lite_PD_rl_dimensions(dims);
      lname  = fullpath;
      reset  = FALSE;
   } else {
      /*
       * If the variable doesn't exist define it to the file.
       */
      number = _lite_PD_comp_num(dims);
      ep     = _lite_PD_mk_syment(outtype, number, addr, NULL, dims);

      strcpy(bf, fullpath);
      lname = lite_SC_firsttok(bf, ".([ ");
      _lite_PD_e_install(lname, ep, file->symtab);

      reset = TRUE;
   }

   if (file->virtual_internal) {
      SC_address ad;

      ad.memaddr = vr;
      ep->blocks->diskaddr = ad.diskaddr;
      lite_SC_mark(vr, 1);
      ep = lite_PD_copy_syment(ep);
   } else {
      if (outtype == NULL) outtype = PD_entry_type(ep);

      if (intype == NULL) intype = outtype;

      /*
       * Go to the correct address.
       */
      if (io_seek(file->stream, addr, SEEK_SET)) {
	 lite_PD_error("FSEEK FAILED TO FIND CURRENT ADDRESS - _PD_WRITE",
		       PD_WRITE);
      }

      /*
       * Do the low level write.
       */
      if (!_lite_PD_hyper_write(file, lname, ep, vr, intype)) {
	 lite_PD_error("CAN'T WRITE VARIABLE - _PD_WRITE", PD_WRITE);
      }

      /*
       * If the variable didn't previously exist we're at the end
       * of the file.
       */
      if (reset) {
	 file->chrtaddr = io_tell(file->stream);
	 if (file->chrtaddr == -1L) {
	    lite_PD_error("CAN'T FIND ADDRESS OF NEXT VARIABLE - _PD_WRITE",
			  PD_WRITE);
	 }

	 /*
	  * Make a releasable copy of the entry
	  * SX depends on this critically!!
	  */
	 ep = lite_PD_copy_syment(ep);
      }
   }

   return(ep);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_create
 *
 * Purpose:	Create a PDB file.
 *
 * Return:	Success:	Initialized PDBfile structure
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 * Mark C. Miller, Fri Apr 13 22:39:29 PDT 2012
 * Pass default creation mode options to PD_mk_pdb().
 *
 * Mark C. Miller, Thu Jun 14 13:25:02 PDT 2012
 * Remove call to io_close in ABORT case. The file pointer may not
 * have been properly initialized.
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
PDBfile *
lite_PD_create (char *name) {

   char str[MAXLINE];
   PDBfile *file;
   static FILE *fp;

   file = NULL;

   switch (setjmp(_lite_PD_create_err)) {
   case ABORT:
      if (fp) io_close(fp);
      return(NULL);
   case ERR_FREE:
      return(file);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * Open the file.
    */
   strncpy(str, name, sizeof(str));
   str[sizeof(str)-1] = '\0';
   fp = io_open(str, BINARY_MODE_WPLUS);
   if (!fp) lite_PD_error("CAN'T CREATE FILE - PD_CREATE", PD_CREATE);

   if (lite_PD_buffer_size != -1) {
      if (io_setvbuf(fp, NULL, _IOFBF, (size_t) lite_PD_buffer_size)) {
	 lite_PD_error("CAN'T SET FILE BUFFER - PD_CREATE", PD_OPEN);
      }
   }

   /*
    * Make the PDBfile.
    */
   file = _lite_PD_mk_pdb(str, lite_PD_DEF_CREATM);
   if (file == NULL) {
      lite_PD_error("CAN'T ALLOCATE PDBFILE - PD_CREATE", PD_OPEN);
   }

   file->stream = fp;
   file->mode   = PD_CREATE;

   /*
    * Set the file data conversion standard - and yes someone might pick
    * a target standard which is the current standard
    */
   file->std   = _lite_PD_copy_standard(file->host_std);
   file->align = _lite_PD_copy_alignment(file->host_align);
   if (lite_REQ_STANDARD != NULL) {
      if (!_lite_PD_compare_std(lite_REQ_STANDARD, file->std,
				lite_REQ_ALIGNMENT, file->align)) {
	 _lite_PD_rl_standard(file->std);
	 file->std   = _lite_PD_copy_standard(lite_REQ_STANDARD);
	 _lite_PD_rl_alignment(file->align);
	 file->align = _lite_PD_copy_alignment(lite_REQ_ALIGNMENT);
      }
      lite_REQ_STANDARD = NULL;
   }

   /*
    * Write the ASCII header.
    */
   io_printf(fp, "%s\n", HeadTok);
   if (io_flush(fp)) {
      lite_PD_error("FFLUSH FAILED BEFORE HEADER - PD_CREATE", PD_CREATE);
   }

   /*
    * Write the primitive data type formats.
    */
   if (!_lite_PD_wr_format(file)) {
      lite_PD_error("FAILED TO WRITE FORMATS - PD_CREATE", PD_CREATE);
   }

   /*
    * Record the current file position as the location of the symbol table
    * address and sequentially the chart address
    */
   if ((file->headaddr = io_tell(fp)) == -1L) {
      lite_PD_error("CAN'T FIND HEADER ADDRESS - PD_CREATE", PD_CREATE);
   }

   /*
    * Initialize the pdb system defs and structure chart.
    */
   _lite_PD_init_chrt(file);

   if (io_flush(fp)) {
      lite_PD_error("FFLUSH FAILED AFTER HEADER - PD_CREATE", PD_CREATE);
   }

   memset(str, 0, PAD_SIZE);
   if (io_write(str, (size_t) 1, PAD_SIZE, fp) != PAD_SIZE) {
      lite_PD_error("FAILED TO PAD FILE FOR MPW - PD_CREATE", PD_CREATE);
   }

   file->chrtaddr = file->headaddr + 128L;
   if (io_seek(fp, file->chrtaddr, SEEK_SET)) {
      lite_PD_error("FAILED TO FIND START OF DATA - PD_CREATE", PD_CREATE);
   }

   file->system_version = PDB_SYSTEM_VERSION;
   file->date           = lite_SC_date();

   return(file);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_defstr
 *
 * Purpose:	A structure definition mechanism for PDB.
 *              sample syntax:                                                 
 *		                                                               
 *		  lite_PD_defstr(<PDB file>, "<struct name>",
 *		                        "<member1>", "<member2>",              
 *		                ...     "<membern>", lite_LAST);
 *		                                                               
 *		where                                                          
 *		                                                               
 *		  <member> := <primitive type> <member name>[(<dimensions>)] | 
 *		              <derived type> <member name>[(<dimensions>)]     
 *		                                                               
 *		  <dimensions> := <non-negative int> |                         
 *		                  <non-negative int>,<dimensions> |            
 *		                  <non-negative int>, <dimensions> |           
 *		                  <non-negative int> <dimensions>              
 *		                                                               
 *		  <primitive type> := short | integer | long | float |         
 *		                      double | char | short * | integer *      
 *		                      long * | float * | double * | char *     
 *		                                                               
 *		  <derived type> := any defstr'd type | any defstr'd type *    
 *		                                                               
 *		lite_LAST is a pointer to a integer zero and is specifically
 *		allocated by PDBLib to be used to terminate argument lists     
 *		which consist of pointers                                      
 *		                                                               
 *		Returns NULL if member types are unknown
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
defstr *
lite_PD_defstr (PDBfile *file, char *name, ...) {

   char 	*nxt, *ptype;
   int 		doffs;
   HASHTAB 	*fchrt;
   memdes 	*desc, *lst, *prev;
   defstr 	*dp;
   va_list	ap ;

   va_start (ap, name);

   prev  = NULL;
   lst   = NULL;
   fchrt = file->chart;
   doffs = file->default_offset;
   for (nxt = va_arg(ap, char*); (int) *nxt != 0; nxt = va_arg(ap, char*)) {
      desc  = _lite_PD_mk_descriptor(nxt, doffs);
      ptype = desc->base_type;
      if (lite_SC_lookup(ptype, fchrt) == NULL) {
	 if ((strcmp(ptype, name) != 0) || !_lite_PD_indirection(nxt)) {
	    sprintf(lite_PD_err, "ERROR: %s BAD MEMBER TYPE - PD_DEFSTR\n",
		    nxt);
	    return(NULL);
	 }
      }

      if (lst == NULL) lst = desc;
      else prev->next = desc;
      prev = desc;
   }

   va_end (ap) ;

   /*
    * Install the type in both charts.
    */
   dp = _lite_PD_defstr_inst(name, lst, -1, NULL, NULL, fchrt,
			     file->host_chart, file->align, file->host_align,
			     FALSE);
   if (dp == NULL) {
      sprintf(lite_PD_err, "ERROR: CAN'T HANDLE PRIMITIVE TYPE - PD_DEFSTR\n");
   }

   return(dp);
}
#endif /* PDB_WRITE */



/*-------------------------------------------------------------------------
 * Function:	lite_PD_cast
 *
 * Purpose:	Tell PDB that the type of a particular member (which must
 *		e a pointer) is specified by another member (which must
 *		be a character pointer)
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
lite_PD_cast (PDBfile *file, char *type, char *memb, char *contr) {

   HASHTAB *tab;
   hashel *hp;
   defstr *dp;
   memdes *desc, *lst;

   /*
    * Add the cast to the file->chart.
    */
   tab = file->chart;
   for (hp = *(tab->table); hp != NULL; hp = hp->next) {
      dp = (defstr *) hp->def;
      if (strcmp(type, dp->type) != 0) continue;

      /*
       * Check that the contr is right.
       */
      for (desc = dp->members; desc != NULL; desc = desc->next) {
	 if (strcmp(contr, desc->name) != 0) continue;

	 /*
	  * Do this once, don't repeat in other chart.
	  */
	 if ((strcmp(desc->base_type, "char") != 0) ||
	     !_lite_PD_indirection(desc->type)) {
	    sprintf(lite_PD_err, "BAD CAST CONTROLLER - PD_CAST");
	    return(FALSE);
	 }
	 break;
      }
   }

   /*
    * Add the cast to the file->host_chart.
    */
   tab = file->host_chart;
   for (hp = *(tab->table); hp != NULL; hp = hp->next) {
      dp = (defstr *) hp->def;
      if (strcmp(type, dp->type) != 0) continue;
      for (desc = dp->members; desc != NULL; desc = desc->next) {
	 if (strcmp(memb, desc->name) != 0) continue;

	 /*
	  * Make an independent copy in case the one in the file
	  * chart is released.
	  */
	 desc->cast_memb = lite_SC_strsavef(contr, "char*:PD_CAST:membh");
	 desc->cast_offs = _lite_PD_member_location(contr, tab, dp, &lst);
      }
   }
   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_target
 *
 * Purpose:	Setup for the target machine data formats and alignments.
 *		This information is recorded in the PDBfiles to correctly
 *		handle things when many files are open at once.
 *		To correctly handle the situation in which there are
 *		PD_OPEN'd files around (this may reset previously set
 *		file->std) remember a standard specifically requested
 *		with PD_TARGET (note that PD_OPEN sets file->std and
 *		file->align).
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Robb Matzke
 *		robb@callisto.matzke.cioe.com
 *		Apr 17, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
lite_PD_target (data_standard *data, data_alignment *align) {

   lite_REQ_STANDARD  = data;
   lite_REQ_ALIGNMENT = align;

   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_flush
 *
 * Purpose:	Dump the data description tables containing the current
 *		state of the PDB file.  The tables are:
 *
 * 		* Structure chart
 * 		* Symbol table
 * 		* Extras table
 * 		
 * 		The table addresses are also updated.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT
 *		Apr 18, 1996
 *
 * Modifications:
 *    Eric Brugger, Mon Dec  7 10:53:08 PST 1998
 *    Removed call to lite_PD_reset_ptr_list since it was removed.
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int
lite_PD_flush (PDBfile *file) {

   FILE *fp;

   if (file->flushed) return(TRUE);

   if (file->attrtab != NULL) {
      lite_PD_cd(file, NULL);
      if (!lite_PD_write(file, PDB_ATTRIBUTE_TABLE, "HASHTAB *",
			 &file->attrtab)) {
	 return(FALSE);
      }
   }

   switch (setjmp(_lite_PD_write_err)) {
   case ABORT:
      return(FALSE);
   case ERR_FREE:
      return(TRUE);
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   fp = file->stream;
   if (io_flush(fp)) {
      lite_PD_error("FFLUSH FAILED BEFORE CHART - PD_FLUSH", PD_WRITE);
   }

   /*
    * Seek the place to write the structure chart.
    */
   if (io_seek(fp, file->chrtaddr, SEEK_SET)) {
      lite_PD_error("FSEEK FAILED TO FIND CHART  - PD_FLUSH", PD_WRITE);
   }

   /*
    * Write the structure chart.
    */
   file->chrtaddr = _lite_PD_wr_chrt(file);
   if (file->chrtaddr == -1L) {
      lite_PD_error("CAN'T WRITE STRUCTURE CHART - PD_FLUSH", PD_WRITE);
   }

   /*
    * Write the symbol table.
    */
   file->symtaddr = _lite_PD_wr_symt(file);
   if (file->symtaddr == -1L) {
      lite_PD_error("CAN'T WRITE SYMBOL TABLE - PD_FLUSH", PD_WRITE);
   }

   /*
    * Write the extras table.
    */
   if (!_lite_PD_wr_extras(file)) {
      lite_PD_error("CAN'T WRITE MISCELLANEOUS DATA - PD_FLUSH", PD_WRITE);
   }

   if (io_tell(fp) == -1L) {
      lite_PD_error("CAN'T FIND HEADER ADDRESS - PD_FLUSH", PD_WRITE);
   }

   if (io_flush(fp)) {
      lite_PD_error("FFLUSH FAILED AFTER CHART - PD_FLUSH", PD_WRITE);
   }

   /*
    * Update the header.
    */
   if (io_seek(fp, file->headaddr, SEEK_SET)) {
      lite_PD_error("FSEEK FAILED - PD_FLUSH", PD_WRITE);
   }

   if (file->headaddr != io_tell(fp)) {
      lite_PD_error("FSEEK FAILED TO FIND HEADER - PD_FLUSH", PD_WRITE);
   }

   io_printf(fp, "%ld\001%ld\001\n", file->chrtaddr, file->symtaddr);

   if (io_flush(fp)) {
      lite_PD_error("FFLUSH FAILED AFTER HEADER - PD_FLUSH", PD_WRITE);
   }

   file->flushed = TRUE;

   return(TRUE);
}
#endif /* PDB_WRITE */


/*-------------------------------------------------------------------------
 * Function:	lite_PD_defent_alt
 *
 * Purpose:	Define an entry in the PDB file symbol table and stake out
 *		the disk space but write nothing.  Dimensional information is
 *		specified by the number of dimensions, ND, and the array of
 *		(min,max) pairs of long ints in IND.
 *
 * Return:	Success:	The new symbol table entry.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT
 *		May  9, 1996
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 15:36:49 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
syment *
lite_PD_defent_alt (PDBfile *file, char *name, char *outtype,
		    int nd, long *ind) {

   int		i;
   long 	number, maxi, mini, leng;
   dimdes	*dims, *next, *prev;

   /*
    * Compute the disk address, the number of items, and the dimensions.
    */
   number = 1L;
   dims   = NULL;
   for (i = 0; i < nd; i++) {
      mini = ind[0];
      maxi = ind[1];
      ind += 2;

      leng    = maxi - mini + 1L;
      number *= leng;

      next = _lite_PD_mk_dimensions(mini, leng);
      if (dims == NULL) {
	 dims = next;
      } else {
	 prev->next = next;
      }

      prev = next;
   }

   return lite_PD_defent (file, name, outtype, number, dims);
}
#endif /* PDB_WRITE */



/*-------------------------------------------------------------------------
 * Function:	lite_PD_defent
 *
 * Purpose:	Define an entry in the PDB file symbol table and
 *              stake out the disk space but write nothing.
 *              any dimensional information is the NAME string.
 *
 * Return:	Success:	New symbol table entry
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT
 *		May  9, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
syment *
lite_PD_defent (PDBfile *file, char *name, char *outtype, long number,
		 dimdes *dims) {

   long		addr, bytespitem;
   defstr	*dp;
   syment	*ep;
   char		bf[MAXLINE], *lname;

   ep = NULL;

   switch (setjmp(_lite_PD_write_err)) {
   case ABORT:
      return(NULL);
   case ERR_FREE:
      return ep;
   default:
      memset(lite_PD_err, 0, MAXLINE);
      break;
   }

   /*
    * If there are pointers involved it is an error.
    */
   dp = PD_inquire_type(file, outtype);
   if (dp == NULL)
      lite_PD_error("UNKNOWN FILE TYPE - _PD_DEFENT", PD_WRITE);

   if (dp->n_indirects) {
      lite_PD_error("CAN'T DEFINE ENTRY WITH INDIRECTS - _PD_DEFENT",
		    PD_WRITE);
   }

   ep = lite_PD_inquire_entry (file, name, FALSE, NULL);

   if (ep == NULL) {
      /*
       * If this is a new entry.
       */
      addr = file->chrtaddr;
      ep   = _lite_PD_mk_syment (outtype, number, addr, NULL, dims);

      strcpy(bf, _lite_PD_fixname(file, name));
      lname = lite_SC_firsttok(bf, ".([ ");
      _lite_PD_e_install(lname, ep, file->symtab);

      bytespitem = _lite_PD_lookup_size(outtype, file->chart);

      ep = _lite_PD_extend_file(file, number*bytespitem) ? ep : NULL;
      
   } else {
      /*
       * If this is only a new block.
       */
      ep = _lite_PD_add_block(file, ep, dims) ? ep : NULL;
   }

   return ep;
}
#endif /* PDB_WRITE */

/*-------------------------------------------------------------------------
 * Function:    lite_PD_entry_number
 *
 * Purpose:     Query entry number        
 *
 * Programmer:  Adapted from PACT, Burl Hall, 26Feb08
 *
 *-------------------------------------------------------------------------
 */
int lite_PD_entry_number(syment* entry) {
    return (PD_entry_number(entry));
}

/*-------------------------------------------------------------------------
 * Function:    lite_PD_get_file_length
 *
 * Purpose:     Return current file size 
 *
 * Programmer:  Adapted from PACT, Mark C. Miller, 26Feb08
 *
 *-------------------------------------------------------------------------
 */
long  lite_PD_get_file_length(PDBfile *file) {
    off_t caddr, flen;

    caddr = io_tell(file->stream);
    io_seek(file->stream, 0, SEEK_END);

    flen = io_tell(file->stream);
    io_seek(file->stream, caddr, SEEK_SET);

    return((long)flen);
}

/*-------------------------------------------------------------------------
 * Function:    lite_PD_append_alt
 *
 * Purpose:      - append a new block of data to an existing entry
 *               - NOTE: VR must be a pointer to an object with the type
 *               - of the existing entry
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT, Burl Hall, 26Feb08
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
int lite_PD_append(PDBfile *file, char *name, void *vr) {
    _append_flag = TRUE;
    return(lite_PD_write_as(file, name, NULL, NULL, vr));
}

int lite_PD_append_alt(PDBfile *file, char *name, void *vr, int nd, long *ind) {
    _append_flag = TRUE;
    return(lite_PD_write_as_alt(file, name, NULL, NULL, vr, nd, ind));
}

int lite_PD_append_as(PDBfile *file, char *name, char *intype, void *vr) {
    _append_flag = TRUE;
    return(lite_PD_write_as(file, name, intype, NULL, vr));
}

int lite_PD_append_as_alt(PDBfile *file, char *name, char *intype,
                     void *vr, int nd, long *ind) {
    _append_flag = TRUE;
    return(lite_PD_write_as_alt(file, name, intype, NULL, vr, nd, ind));
}
#endif /* PDB_WRITE */

/*-------------------------------------------------------------------------
 * Function:    lite_PD_set_major_order
 *
 * Purpose:     Set major storage order.
 *
 * Return:      void
 *
 * Programmer:  Adapted from PACT, Burl Hall, 26Feb08
 *
 *-------------------------------------------------------------------------
 */
#ifdef PDB_WRITE
void lite_PD_set_major_order( PDBfile* file, int type) {
  PD_set_major_order( file, type ) ;
}
#endif /* PDB_WRITE */
