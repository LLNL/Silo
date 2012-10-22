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
#include "config.h" /* For HAVE_MEMMOVE test. */
/*
 * SCCTLA.C - auxilliary functions for control structure package
 *          - these should NEVER be modified for a specific application
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 *
 */
#include "config.h"
#include <score.h>
#include <limits.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>

typedef union u_mem_header mem_header;
typedef struct s_mem_descriptor mem_descriptor;

#define SC_MEM_ID       0xF1E2D3C4
#define UNIT_SIZE_MAX   10

/*
 * This parameter controls wether small requests for memory go to
 * the system memory manager or are allocated out of a larger chunks
 * of memory which score manages.
 */
#undef CACHE_SMALL_MEM_REQ

/*
 * The struct s_mem_descriptor will contain prev and next fields if
 * NEED_MEM_TRACE is defined and the macros ASSIGN_BLOCK, SAVE_LINKS,
 * REASSIGN_BLOCK, UNASSIGN_BLOCK and the function lite_SC_realloc() use
 * these fields.  These machines need this defined: irix, linux.
 * These machines might get away with not defining this: meiko, solaris,
 * sun, sun-gnu.
 */
#if NORMAL_SCLITE_HEADERS
#define NEED_MEM_TRACE
#endif
#ifdef NEED_MEM_TRACE

/*
 * Use the mem_header struct to force alignment to that of a double
 * this solves all alignment problems (especially for RISC chips).
 */

struct s_mem_descriptor {
   char			*name;
   long 		id;
   short 		ref_count;
   short 		type;
   long 		length;
   mem_header 		*prev;
   mem_header 		*next;
};

union u_mem_header {
   mem_descriptor	block;
   double 		align[3];
};

static mem_header	*_SC_latest_block ;

#define ASSIGN_BLOCK(space, nb, name) {                                \
      mem_descriptor *desc;                                            \
      desc = &space->block;                                            \
      desc->id        = SC_MEM_ID;                                     \
      desc->ref_count = 1;                                             \
      desc->type      = 0;                                             \
      desc->length    = nb;                                            \
      desc->name      = name;                                          \
      if (_SC_latest_block != NULL) {                                  \
	 mem_header *prev, *next;                                      \
	 next = _SC_latest_block->block.next;                          \
	 prev = _SC_latest_block;                                      \
	 next->block.prev = space;                                     \
	 prev->block.next = space;                                     \
	 desc->next = next;                                            \
	 desc->prev = prev;                                            \
	 _SC_latest_block = space;                                     \
      } else {                                                         \
	 _SC_latest_block             = space;                         \
	 _SC_latest_block->block.prev = space;                         \
	 _SC_latest_block->block.next = space;			       \
      }								       \
   }

#define SAVE_LINKS(desc) {                                             \
      prev = desc->prev;                                               \
      next = desc->next;                                               \
      if (space == _SC_latest_block) _SC_latest_block = next;          \
   }

#define REASSIGN_BLOCK(space) {                                        \
       desc = &space->block;                                           \
       desc->length = nb;                                              \
       prev->block.next = space;                                       \
       next->block.prev = space;				       \
    }

#define UNASSIGN_BLOCK(desc) {                                         \
       mem_header *prev, *next;                                        \
       prev = desc->prev;                                              \
       next = desc->next;                                              \
       if (prev == next) _SC_latest_block = NULL;                      \
       prev->block.next = next;                                        \
       next->block.prev = prev;                                        \
       if (space == _SC_latest_block) _SC_latest_block = next;         \
    }

#else /* NEED_MEM_TRACE */

/*
 * Use the mem_header struct to force alignment to that of a double
 * this solves all alignment problems (especially for RISC chips)
 */
struct s_mem_descriptor {
   short 		ref_count;
   int                  length;
};

union u_mem_header {
   mem_descriptor 	block;
   double 		align[1];
};

#define ASSIGN_BLOCK(space, nb, name) {                                \
      mem_descriptor *desc;                                            \
      desc = &space->block;                                            \
      desc->ref_count = 1;                                             \
      desc->length    = nb;                                            \
   }

#define SAVE_LINKS(desc) /*void*/

#define REASSIGN_BLOCK(space) {                                        \
       desc = &space->block;                                           \
       desc->length = nb;					       \
    }

#define UNASSIGN_BLOCK(desc) /*void*/

#endif /* NEED_MEM_TRACE */

#define SCORE_BLOCK_P(desc)                                            \
    (1 == 1)

#define BLOCK_LENGTH(desc)                                             \
    (desc)->length

#define REF_COUNT(desc)                                                \
    ((desc)->ref_count)


#define EXTRA_WORD_SIZE sizeof(long)
#define NBITS (8*EXTRA_WORD_SIZE - 4)

#define _SC_REALLOC (*_lite_SC_realloc_hook)
#define _SC_ALLOC   (*_lite_SC_alloc_hook)
#define _SC_FREE    (*_lite_SC_free_hook)

#define SC_MEM_COUNT                                                   \
    lite_SC_c_sp_diff = lite_SC_c_sp_alloc - lite_SC_c_sp_free;        \
    lite_SC_c_sp_max  = (lite_SC_c_sp_max > lite_SC_c_sp_diff) ?       \
                   lite_SC_c_sp_max : lite_SC_c_sp_diff

FreeFuncType   		_lite_SC_free_hook    = (FreeFuncType) free;
ReallocFuncType		_lite_SC_realloc_hook = (ReallocFuncType) realloc;
MallocFuncType		_lite_SC_alloc_hook   = (MallocFuncType) malloc;


#ifdef CACHE_SMALL_MEM_REQ
static mem_descriptor	 *_SC_free_list[UNIT_SIZE_MAX];
static char		**_SC_major_block_list ;
static long		_SC_nx_major_blocks ;
static long	 	_SC_n_major_blocks ;
#endif

static long		_SC_max_mem_blocks ;
static long		_SC_n_mem_blocks ;
static unsigned long	Sz_max = ( 1L << NBITS) - 1;        /* maximum value */
static unsigned long	Sz = sizeof(mem_header);  /* size in bytes of header */
static char		tokbuffer[MAXLINE];  /* used by firsttok and lasttok */
static int		_SC_zero_space = TRUE;
static lite_SC_byte *		_SC_prim_alloc (size_t) ;
static void		_SC_prim_free (lite_SC_byte*,unsigned long) ;


/*-------------------------------------------------------------------------
 * Function:	lite_SC_alloc
 *
 * Purpose:	Add a layer of control over the C level mamory management
 *		system to store the byte length of allocated spaces.
 *		A space EXTRA_WORD_SIZE greater than requested is allocated.
 *		The length in bytes is written into the first EXTRA_WORD_SIZE
 *		bytes with a 4 bit marker in the high bits and a pointer
 *		to the next byte is returned.
 *
 * Return:	Success:	Ptr to user memory
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
lite_SC_byte *
lite_SC_alloc (long nitems, long bytepitem, char *name) {

   long nb;
   unsigned long nbp;
   mem_header *space;

   nb = nitems*bytepitem;

   if ((nb <= 0) || (nb > Sz_max)) return(NULL);

   nbp = (unsigned long) nb + Sz;
   space = (mem_header *) _SC_prim_alloc((size_t) nbp);

   if (space != NULL) {
      ASSIGN_BLOCK(space, nb, name);

      lite_SC_c_sp_alloc += nb;
      SC_MEM_COUNT;
    
      _SC_max_mem_blocks++;
      _SC_n_mem_blocks++;

      space++;

      /*
       * Zero out the space.
       */
      if (_SC_zero_space) memset(space, 0, nb);
   }

   return((lite_SC_byte *) space);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_realloc
 *
 * Purpose:	Add a layer of control over the C level memory management
 *		system to store the byte length of allocated spaces.  A
 *		space EXTRA_WORD_SIZE greater than requested is reallocated.
 *		The length in bytes is written into the first EXTRA_WORD_SIZE
 *		bytes with a 4 bit marker in the high bits and a pointer to
 *		the next byte is returned.  
 *
 * Return:	Success:	Ptr to move memory.
 *
 *		Failure:	If the maximum size implied by the
 *				EXTRA_WORD_SIZE-4 is exceeded a NULL
 *				ptr is returned.
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
lite_SC_byte *
lite_SC_realloc (lite_SC_byte *p, long nitems, long bytepitem) {

   long nb, ob, db;
   unsigned long nbp, obp;
   mem_header *space, *tmp;
   mem_descriptor *desc;

#ifdef NEED_MEM_TRACE
   mem_header *prev, *next;
#endif

   if (p == NULL) return(NULL);

   space = ((mem_header *) p) - 1;
   desc  = &space->block;
   if (!SCORE_BLOCK_P(desc)) return(NULL);

   nb  = nitems*bytepitem;
   nbp = nb + Sz;

   if ((nb <= 0) || (nb > Sz_max)) return(NULL);

   ob = lite_SC_arrlen(p);
   db = nb - ob;
    
   SAVE_LINKS(desc);

   tmp = (mem_header *) _SC_prim_alloc((size_t) nbp);
   obp = ob + Sz;
   memcpy(tmp, space, MIN(obp, nbp));
   _SC_prim_free(space, obp);
   space = tmp;
    
   if (space != NULL) {
      REASSIGN_BLOCK(space);

      lite_SC_c_sp_alloc += db;
      SC_MEM_COUNT;

      space++;

      /*
       * Zero out the new space.
       */
      if ((db > 0) && (_SC_zero_space)) memset(((char *) space + ob), 0, db);
   }

   return((lite_SC_byte *) space);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_free
 *
 * Purpose:	The complementary routine for lite_SC_alloc().  Free all
 *		the space including the counter.
 *
 * Return:	Success:	TRUE
 *
 *		Failure:	FALSE
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_SC_free (lite_SC_byte *p) {

   mem_header *space;
   mem_descriptor *desc;
   unsigned long nbp;

   if (p == NULL) return(TRUE);

   space = ((mem_header *) p) - 1;
   desc  = &space->block;
   if (!SCORE_BLOCK_P(desc)) return(FALSE);

   if (REF_COUNT(desc) == UNCOLLECT) return(TRUE);

   if (--REF_COUNT(desc) > 0) return(TRUE);

   nbp = BLOCK_LENGTH(desc) + Sz;

   UNASSIGN_BLOCK(desc);

   lite_SC_c_sp_free += (nbp - Sz);
   SC_MEM_COUNT;

   if (_SC_zero_space) {
      memset(space, 0, nbp);
   } else {
#ifdef NEED_MEM_TRACE
      desc->name      = NULL;
      desc->id        = 0L;
      desc->type      = 0;
#endif
      desc->ref_count = 0;
      desc->length    = 0L;
   }

   _SC_prim_free((lite_SC_byte *) space, nbp);
   _SC_n_mem_blocks--;
   return(TRUE);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_arrlen
 *
 * Purpose:	Return the length of an array which was allocated
 *		with lite_SC_alloc().
 *
 * Return:	Success:	Length of array.
 *
 *		Failure:	-1
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
long
lite_SC_arrlen (lite_SC_byte *p) {

   mem_header *space;
   mem_descriptor *desc;
   long nb;

   if (p == NULL) return(-1);

   space = ((mem_header *) p) - 1;
   desc  = &space->block;
   if (!SCORE_BLOCK_P(desc)) return(-1L);

   nb = BLOCK_LENGTH(desc);
   if (nb < 0L) return(-1L);
   else return(nb);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_mark
 *
 * Purpose:	Change the reference count by N
 *
 * Return:	Success:	New reference count.
 *
 *		Failure:	-1
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_SC_mark (lite_SC_byte *p, int n) {

   mem_header *space;
   mem_descriptor *desc;

   if (p == NULL) return(-1);

   space = ((mem_header *) p) - 1;
   desc  = &space->block;
   if (!SCORE_BLOCK_P(desc)) return(-1);

   if (REF_COUNT(desc) < UNCOLLECT) REF_COUNT(desc) += n;

   return(REF_COUNT(desc));
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_ref_count
 *
 * Purpose:	Reference count.
 *
 * Return:	Success:	Return the reference count of the given
 *				object.
 *
 *		Failure:	-1
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
lite_SC_ref_count (lite_SC_byte *p) {

   mem_header *space;
   mem_descriptor *desc;

   if (p == NULL) return(-1);

   space = ((mem_header *) p) - 1;
   desc  = &space->block;
   if (!SCORE_BLOCK_P(desc)) return(-1);

   return((int) REF_COUNT(desc));
}


/*-------------------------------------------------------------------------
 * Function:	_SC_prim_alloc
 *
 * Purpose:	Memory allocator that manages banks of small chunks
 *		for efficiency.
 *
 * Return:	Success:	Ptr to user memory
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *    Eric Brugger, Thu Dec 10 12:00:55 PST 1998
 *    I made the caching of small memory chunks depend on a C preprocessor
 *    variable.
 *
 *-------------------------------------------------------------------------
 */
static lite_SC_byte *
_SC_prim_alloc (size_t nbp) {

   lite_SC_byte *p;
#ifdef CACHE_SMALL_MEM_REQ
   char *pn;
   mem_descriptor *md, *ths;
   size_t tnb;
   long nb, unsz;
   int nu, us, ns, i;
#endif

   if (nbp <= 0) return(NULL);

#ifdef CACHE_SMALL_MEM_REQ
   nb   = nbp - Sz;
   unsz = nb >> 3;
   if (unsz < UNIT_SIZE_MAX) {
      md = _SC_free_list[unsz];
      if (md == NULL) {
	 us = Sz + ((unsz + 1) << 3);
	 nu = 4096/us;
	 ns = nu*us;
	 pn = _SC_ALLOC((size_t) ns);

	 /*
	  * SC_REMEMBER would be nice but it would also be a recursive
	  * infinite loop.
	  */
	 if (_SC_major_block_list == NULL) {
	    _SC_nx_major_blocks = 10L;
	    _SC_n_major_blocks  = 0;
	    tnb = sizeof(char *)*_SC_nx_major_blocks;
	    _SC_major_block_list = (char **) _SC_ALLOC(tnb);
	 }

	 _SC_major_block_list[_SC_n_major_blocks++] = pn;

	 if (_SC_n_major_blocks >= _SC_nx_major_blocks) {
	    _SC_nx_major_blocks += 10L;
	    tnb = sizeof(char *)*_SC_nx_major_blocks;
	    _SC_major_block_list = (char **) _SC_REALLOC(_SC_major_block_list,
							 tnb);
	 }

	 md = (mem_descriptor *) pn;
	 nu--;
	 for (i = 0; i < nu; i++, pn += us) {
	    ths       = (mem_descriptor *) pn;
#ifdef NEED_MEM_TRACE
	    ths->name = (char *) (pn + us);
#endif
	 }
	 ths       = (mem_descriptor *) pn;
#ifdef NEED_MEM_TRACE
	 ths->name = NULL;
#endif
      }

#ifdef NEED_MEM_TRACE
#error HOW TO UPDATE FREE LIST IF THIS CODE IS ENABLED 
      _SC_free_list[unsz] = (mem_descriptor *) (md->name);
#endif
      p = (lite_SC_byte *) md;
   } else {
      p = _SC_ALLOC((size_t) nbp);
   }
#else
   p = _SC_ALLOC((size_t) nbp);
#endif

   return(p);
}


/*-------------------------------------------------------------------------
 * Function:	_SC_prim_free
 *
 * Purpose:	Free small block counterpart to _SC_prim_alloc()
 *		for efficiency.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *    Eric Brugger, Thu Dec 10 12:00:55 PST 1998
 *    I made the caching of small memory chunks depend on a C preprocessor
 *    variable.
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static void
_SC_prim_free (lite_SC_byte *p, unsigned long nbp) {

#ifdef CACHE_SMALL_MEM_REQ
   mem_descriptor *lst, *ths;
   unsigned long nb, unsz;
#endif

   if (p == NULL) return;

#ifdef CACHE_SMALL_MEM_REQ
   nb   = nbp - Sz;
   unsz = nb >> 3;
   if (unsz < UNIT_SIZE_MAX) {
      ths = (mem_descriptor *) p;
      lst = _SC_free_list[unsz];
#ifdef NEED_MEM_TRACE
      ths->name = (char *) lst;
#endif
      _SC_free_list[unsz] = ths;
   } else {
      _SC_FREE(p);
   }
#else
   _SC_FREE(p);
#endif
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_strsavef
 *
 * Purpose:	Save string S somewhere.  Remember its name.  Allocate
 *		one extra character so that firsttok won't kill things
 *		in the one bad case.
 *
 * Return:	Success:	New memory for string.
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_SC_strsavef (char *s, char *name) {

   char *p;
   int sz;

   if (s == NULL) return(NULL);

   sz = strlen(s) + 2;
   p  = FMAKE_N(char, sz, name);
   if (p != NULL) strcpy(p, s);
   else return(NULL);

   return(p);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_strrev
 *
 * Purpose:	Copy the string onto itself in reverse order.
 *
 * Return:	Success:	The string
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_SC_strrev (char *s) {

   int i;
   char *t, *p;

   p = s;
   i = strlen(s) + 1;
   t = MAKE_N(char, i);
   if (t == NULL) return(NULL);

   t[--i] = '\0';
   while (*p) t[--i] = *p++;

   strcpy(s, t);
   SFREE(t);

   return(s);
}

/*--------------------------------------------------------------------------*/
/*                           TOKENIZERS                                     */
/*--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 * Function:	lite_SC_firsttok
 *
 * Purpose:	Returns a pointer to the first token and points
 *		S to the next element in the string.
 *
 * Return:	Success:	Ptr to first token.
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_SC_firsttok (char *s, char *delim) {

   char *t, *r;
        
   if (*s == '\0') return(NULL);

   /*
    * T is the pointer to the token.
    */
   for (t = s; strchr(delim, *t) != NULL; t++) {
      if (*t == '\0') return(NULL);
   }

   /*
    * R is the pointer to the remainder.
    */
   for (r = t; strchr(delim, *r) == NULL; r++) /*void*/ ;

   /*
    * If we aren't at the end of the string.
    */
   if (*r != '\0') {
      *r++ = '\0';

      /*
       * Copy the token into a temporary.
       */
      strcpy(tokbuffer, t);

      /*
       * Copy the remainder down into the original string
       * GOTCHA: this should be replaced by MEMMOVE (ANSI standard C function)
       */
      /*strcpy(s, r);*/
      memmove(s, r, strlen(r)+1);

      /*
       * Copy the token in the space left over.
       */
      t = s + strlen(s) + 1;
      strcpy(t, tokbuffer);
      
   } else {
      /*
       * If we are at the end of the string we may overindex the string
       * by adding one more character (sigh).
       */
      strcpy(tokbuffer, t);
      *s = '\0';
      t = s + 1;
      strcpy(t, tokbuffer);
   }

   return(t);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_SC_pr_tok
 *
 * Purpose:	Returns a pointer to the first token and points S to
 *		the next element in the string.
 *
 * Return:	Success:	First token in S
 *
 *		Failure:	NULL
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
_lite_SC_pr_tok (char *s, char *delim) {

   int i, j;
        
   i = strcspn(s, delim);
   j = strlen(s);
   if ((i == 0) && (i != j)) {
      s++;
      return(lite_SC_firsttok(s, delim));
   }

   s[i] = '\0';
   strcpy(tokbuffer, s);

   /*
    * Take care of last token in string.
    */
   if (i == j) *s = '\0';
   else strcpy(s, s+i+1);

   s += strlen(s) + 1;
   strcpy(s, tokbuffer);

   return(s);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_lasttok
 *
 * Purpose:	Find last token on a string, return it and the
 *		preceeding string.
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
lite_SC_lasttok (char *s, char *delim) {

   char *temp, *r;

   r    = lite_SC_strrev(s);
   temp = _lite_SC_pr_tok(r, delim);
   s    = lite_SC_strrev(r);

   return lite_SC_strrev (temp);
}
