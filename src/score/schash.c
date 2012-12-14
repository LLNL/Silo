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
 * SCHASH.C - routines to manipulate hash tables
 *          - intended to be a complete module that
 *          - can be used with any application by defining
 *          - the struct, hashel, in SCORE.H suitably
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 */
#include "score.h"

/*-------------------------------------------------------------------------
  Function: bjhash 

  Purpose: Hash a variable length stream of bytes into a 32-bit value.

  Programmer: By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.

  You may use this code any way you wish, private, educational, or
  commercial.  It's free. However, do NOT use for cryptographic purposes.

  See http://burtleburtle.net/bob/hash/evahash.html
 *-------------------------------------------------------------------------*/

#define bjhash_mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

#define K(I,L)    (((unsigned int)((unsigned char)k[I]))<<L)
static unsigned int bjhash(register const char *k,
    register unsigned int length, register unsigned int initval)
{
   register unsigned int a,b,c,len;

   len = length;
   a = b = 0x9e3779b9;
   c = initval;

   while (len >= 12)
   {
      a += (K(0,0) + K(1,8) + K(2,16) + K(3,24));
      b += (K(4,0) + K(5,8) + K(6,16) + K(7,24));
      c += (K(8,0) + K(9,8) + K(10,16) + K(11,24));
      bjhash_mix(a,b,c);
      k += 12; len -= 12;
   }

   c += length;

   switch(len)
   {
      case 11: c+=K(10,24);
      case 10: c+=K(9,16);
      case 9 : c+=K(8,8);
      case 8 : b+=K(7,24);
      case 7 : b+=K(6,16);
      case 6 : b+=K(5,8);
      case 5 : b+=K(4,0);
      case 4 : a+=K(3,24);
      case 3 : a+=K(2,16);
      case 2 : a+=K(1,8);
      case 1 : a+=K(0,0);
   }

   bjhash_mix(a,b,c);

   return c;
}
#undef K
#undef bjhash_mix


/*-------------------------------------------------------------------------
 * Function:	lite_SC_hash
 *
 * Purpose:	Compute hash value for string S in a table of SIZE
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
 *   Mark C. Miller, Fri Apr 13 22:43:19 PDT 2012
 *   Use "BJ Hash"
 *-------------------------------------------------------------------------
 */
int
lite_SC_hash (char *s, int size) {

   int len = strlen(s);
   unsigned int hashval = bjhash(s, len, 0xDeadBeef);
   if (hashval > INT_MAX) hashval -= INT_MAX;
   return hashval % size;
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_lookup
 *
 * Purpose:	Lookup S in hash table TABLE.
 *
 * Return:	Success:	Ptr to object
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
hashel *
lite_SC_lookup (char *s, HASHTAB *tab) {

   hashel *np, **tb;
   int sz;

   if (tab == NULL) return(NULL);

   sz = tab->size;
   tb = tab->table;
   for (np = tb[lite_SC_hash(s, sz)]; np != NULL; np = np->next) {
      if (strcmp(s, np->name) == 0) return(np); /* found it */
   }

   return(NULL); /* not found */
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_def_lookup
 *
 * Purpose:	Return a ptr to the object version of LOOKUP.
 *
 * Return:	Success:	ptr to the object version of LOOKUP.
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
lite_SC_def_lookup (char *s, HASHTAB *tab) {

   hashel *np;
   
   if (tab == NULL) return(NULL);

   np = lite_SC_lookup(s, tab);
   if (np != NULL) return(np->def);
   else return(NULL);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_install
 *
 * Purpose:	
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
hashel *
lite_SC_install (char *name, lite_SC_byte *obj, char *type, HASHTAB *tab) {

   return _lite_SC_install (name, obj, type, tab);
}


/*-------------------------------------------------------------------------
 * Function:	_lite_SC_install
 *
 * Purpose:	Install an object in the hash table.  The object type is
 *		defined in hash.h and is generic to enhance the
 *		portability of this code.  WARNING: do NOT use litereals
 *		or volatiles for the type--for efficiency they are
 *		not strsave'd!!!
 *
 * Return:	Success:	
 *
 *		Failure:	
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *    Eric Brugger, Tue Dec  8 16:57:20 PST 1998
 *    Remove unnecessary calls to lite_SC_mark, since reference count now
 *    set when allocated.  The mark flag is now rendered useless since
 *    the memory will always be marked when it is allocated.
 *
 *    Eric Brugger, Thu Sep 23 10:16:30 PDT 1999
 *    Remove the mark flag from the argument list.
 *
 *-------------------------------------------------------------------------
 */
hashel *
_lite_SC_install (char *name, lite_SC_byte *obj, char *type, HASHTAB *tab) {

   hashel *np, **tb;
   int hashval, sz;

   sz = tab->size;
   tb = tab->table;
   np = lite_SC_lookup(name, tab);

   /*
    * If not found install it.
    */
   if (np == NULL) {
      np = FMAKE(hashel, "SC_INSTALL:np");
      if (np == NULL) return(NULL);

      np->name = lite_SC_strsavef(name, "char*:SC_INSTALL:name");
      if (np->name == NULL) return(NULL);

      hashval     = lite_SC_hash(np->name, sz);
      np->next    = tb[hashval];
      tb[hashval] = np;
      (tab->nelements)++;
   }

   np->type = type;
   np->def  = obj;

   return(np);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_hash_rem
 *
 * Purpose:	Remove the specified entry from the given hash table.
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
lite_SC_hash_rem (char *name, HASHTAB *tab) {

   hashel *np, *nxt, **tb;
   int sz, i;

   sz = tab->size;
   tb = tab->table;
   i  = lite_SC_hash(name, sz);
   np = tb[i];

   /*
    * If not found nothing else to do.
    */
   if (np == NULL) {
      return(FALSE);
   } else {
      if (strcmp(name, np->name) == 0) {
	 tb[i] = np->next;

	 /*
	  * Undo the MARK in SC_install.
	  */
	 SFREE(np->def);
	 SFREE(np->name);
	 SFREE(np);
	 (tab->nelements)--;
	 return(TRUE);

      } else {
	 /*
	  * Otherwise search for it.
	  */
	 for (; np->next != NULL; np = np->next) {
	    nxt = np->next;
	    if (strcmp(name, nxt->name) == 0) {
	       np->next = nxt->next;
	       SFREE(nxt->def);
	       SFREE(nxt->name);
	       SFREE(nxt);
	       (tab->nelements)--;
	       return(TRUE);
	    }
	 }
      }
   }
   return(FALSE);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_hash_clr
 *
 * Purpose:	Clear the specified hash table.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
lite_SC_hash_clr (HASHTAB *tab) {

   int i, sz;
   hashel **tb, *np, *nxt;

   sz = tab->size;
   tb = tab->table;
   for (i = 0; i < sz; i++) {
      for (np = tb[i]; np != NULL; np = nxt) {
	 nxt = np->next;

	 /*
	  * Undo the MARK in SC_install.
	  */
	 SFREE(np->def);
	 SFREE(np->name);
	 SFREE(np);
      }
      tb[i] = NULL;
   }
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_make_hash_table
 *
 * Purpose:	Allocate and initialize a hash table of size SZ.
 *
 * Return:	Success:	HASHTAB pointer
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
HASHTAB *
lite_SC_make_hash_table (int sz, int docflag) {

   HASHTAB *tab;
   hashel **tb;
   int i;

   /*
    * Allocate a new hash table.
    */
   tab = FMAKE(HASHTAB, "SC_MAKE_HASH_TABLE:tab");

   if (tab == NULL) {
      printf("\nCannot allocate a new hash table of size %d\n", sz);
      return(NULL);
   }

   tb = FMAKE_N(hashel *, sz, "SC_MAKE_HASH_TABLE:tb");
   if (tb == NULL) return(NULL);

   tab->size      = sz;
   tab->docp      = docflag;
   tab->nelements = 0;
   tab->table     = tb;

   /*
    * Explicitly NULL the pointers.
    */
   for (i = 0; i < sz; i++) tb[i] = NULL;

   return(tab);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_rl_hash_table
 *
 * Purpose:	Release a hash table.  Call SC_HASH_CLR first to
 *		release the contents of the table.
 *
 * Return:	void
 *
 * Programmer:	Adapted from PACT SCORE
 *		Mar 12, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
lite_SC_rl_hash_table (HASHTAB *tab) {

   lite_SC_hash_clr(tab);

   SFREE(tab->table);
   SFREE(tab);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_hash_dump
 *
 * Purpose:	Return a array of pointers whose entries point to the
 *		installed names in the given hash table and are alphabetically
 *		ordered by strcmp().
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
char **
lite_SC_hash_dump (HASHTAB *tab, char *patt) {
   
   return lite_SC_dump_hash (tab, patt, TRUE);
}


/*-------------------------------------------------------------------------
 * Function:	lite_SC_dump_hash
 *
 * Purpose:	Return an array of pointers whose entries point to the
 *		installed names in the given hash table.
 *
 * Return:	Success:	Array of ptrs to entries.
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
char **
lite_SC_dump_hash (HASHTAB *tab, char *patt, int sort) {

   hashel *np, **tb;
   char **lineptr, *name;
   int i, sz, nlines;

   if (tab == NULL) return(NULL);

   /*
    * Allocate a list of pointers to the names in the hash table.
    */
   lineptr = FMAKE_N(char *, tab->nelements, "SC_HASH_DUMP:lineptr");
   if (lineptr == NULL) return(NULL);

   /*
    * Fill in the list of pointers to names in the hash table.
    */
   sz = tab->size;
   tb = tab->table;
   nlines = 0;
   for (i = 0; i < sz; i++) {
      for (np = tb[i]; np != NULL; np = np->next) {
	 name = np->name;
	 if (patt == NULL) lineptr[nlines++] = name;
	 else if (lite_SC_regx_match(name, patt)) lineptr[nlines++] = name;
      }
   }

   /*
    * Check that the number of names found is what is expected.
    */
   if (nlines > tab->nelements) return(NULL);

   REMAKE_N(lineptr, char *, nlines + 1);
   lineptr[nlines] = NULL;

   /*
    * Sort the names.
    */
   if (sort) lite_SC_string_sort(lineptr, nlines);

   /*
    * Return the list of names.
    */
   return(lineptr);
}
