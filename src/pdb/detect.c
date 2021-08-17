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
 * DETECT.C - decipher the basic C binary data formats
 *          - adapted and completed from the Dave Munro original
 *          - this is constructed to be absolutely vanilla C and
 *          - should compile correctly with ANSI, PCC, and many Joe's
 *          - compilers
 *
 * Source Version: 9.0
 * Software Release #92-0043
 *
 *   Mark C. Miller, Fri Nov 13 15:33:42 PST 2009
 *   Added support for long long datatype.
 *
 *   Mark C. Miller, Tue Nov 17 22:23:42 PST 2009
 *   Changed support for long long to match more closely what PDB
 *   proper does.
 */

#include "config.h"
#include <stdio.h>
#if HAVE_STRING_H
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* DIFFER_BYTE - find the location of the differing byte */

int differ_byte( int n, int k, int *fb, unsigned char *a, unsigned char *b)
{int i;

fb[k] = 0;

for (i = 0; i < n; i++)
   {if (a[i] != b[i])
      {fb[k] = i + 1;
      break;};};

return(i);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* FIX_ORDER - determine the byte order assuming that the mantissa byte
 *           - pattern implies the entire ordering
 */

int fix_order ( int n, int *fb, int first, int last )
{int i;
static int ordering = -1;

if ((ordering == -1) && (last > first + 1))

/* left to right or normal ordering */
   {if ((fb[last] > fb[last-1]) && (fb[last-1] > fb[last-2]))
      ordering = 0;

/* right to left or reverse ordering (INTEL) */
   else if ((fb[last] < fb[last-1]) && (fb[last-1] < fb[last-2]))
      ordering = 1;

/* middle endian machine (VAX) */
   else
      ordering = 2;};

switch (ordering)

/* left to right or normal ordering */
   {case 0 :
      for (i = 0; i < n; i++)
	 fb[i] = i + 1;
   break;

/* right to left or reverse ordering (INTEL) */
   case 1 :
      for (i = 0; i < n; i++)
	 fb[i] = n - i;
      break;

/* middle endian machine (VAX)
 * NOTE: this assumes that only the VAX does this!
 */
   case 2 :
      for (i = 0; i < n; i += 2)
	 {fb[i]   = n + 2;
	 fb[i+1] = n + 1;};
      break;};

return(1);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* GUARD_BIT - check for the existence of a mantissa guard bit */

int guard_bit ( int n, int k, int *fb, long *ff, unsigned char *a
              , unsigned char *b
              )
{int i, j, l;
unsigned char byt, nxt;

ff[k] = 0L;

/* xor the two arrays - they should differ by one bit */
for (i = 0; i < n; i++)
   {l   = fb[i] - 1;
   byt = a[l] ^ b[l];
   if (byt == 1)
      {if (b[l+1])
	 {ff[k] = 1L;
	 return(1);};}

   else if (byt != 0)

/* find the location of the bit */
      for (j = 0; j < 8; j++)
	 {if (byt == 0x80)

/* if the next bit of the second one is on there is a guard bit */
	    {nxt = b[l] << (j+1);
	    if (nxt)
	       {ff[k] = 1L;
	       return(1);};};

	 byt <<= 1;};};

return(0);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* DIFFER_BIT - find the location of the differing bit */

int differ_bit ( int n, int k, int *fb, long *ff, unsigned char *a
               , unsigned char *b
               )
{unsigned char byt;
int i, j, l;

for (i = 0; i < n; i++)
   {l = fb[i] - 1;
   if (a[l] != b[l])
      {byt   = a[l] ^ b[l];
      for (j = 0; j < 8; j++)
	 {if (byt == 0x80)
	    {ff[k] = j + i*8;
	    break;};
	 byt <<= 1;};};};

return(1);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* FIND_EXP - find the exponent bias */

int find_exp ( int *fb, long *ff, unsigned char *a )
{int i, l, nbits, sbyte, ebyte;
long exp;

sbyte = ff[4]/8;
ebyte = ff[5]/8;
exp   = 0L;
for (i = sbyte; i <= ebyte; i++)
   {l = fb[i] - 1;
   exp <<= 8;
   exp |= a[l];};

nbits = (ebyte + 1)*8 - ff[5];
exp >>= nbits;

ff[7] = exp;

return(1);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* DERIVE_FP_FORMAT - figure out the floating point format */

int derive_fp_format ( int *fb, int *db, long *ff, long *df )
{int j, szf, szd, first, last;
union ucf {unsigned char c[32]; float f[2];} bofa, bofb, bofc;
union ucd {unsigned char c[32]; double d[2];} boda, bodb, bodc;
float fval, dfv;
double dval, ddv;

szd = sizeof(double);
szf = sizeof(float);

df[0] = szd*8;
ff[0] = szf*8;

/* get the byte ordering from the double mantissa bytes */
dval  = 0.0;
ddv   = 1.0;
first = -1;
last = 2;
for (j = 1; j < szd; j++)
   {bodb.d[0] = dval;

   dval += ddv;
   ddv  /= 256.0;
   boda.d[0] = dval;

   if (j > 1)
      {if (differ_byte(szd, j, db, bodb.c, boda.c) < szd)
	 last = j;
      if (first == -1)
	 first = last;};};

fix_order(szd, db, first, last);

/* get the byte ordering from the float mantissa bytes */
fval  = 0.0;
dfv   = 1.0;
first = -1;
for (j = 1; j < szf; j++)
   {bofb.f[0] = fval;

   fval += dfv;
   dfv  /= 256.0;
   bofa.f[0] = fval;

   if (j > 1)
      {if (differ_byte(szf, j, fb, bofb.c, bofa.c) < szf)
	 last = j;
      if (first == -1)
	 first = last;};};

fix_order(szf, fb, first, last);

/* find the mantissa guard bit for existence */
boda.d[0] = 0.5;
bodb.d[0] = 1.0;
guard_bit(szd, 6, db, df, boda.c, bodb.c);

bofa.f[0] = 0.5;
bofb.f[0] = 1.0;
guard_bit(szf, 6, fb, ff, bofa.c, bofb.c);

/* find the sign bit */
boda.d[0] = -1.0;
bodb.d[0] = 1.0;
differ_bit(szd, 3, db, df, boda.c, bodb.c);

bofa.f[0] = -1.0;
bofb.f[0] = 1.0;
differ_bit(szf, 3, fb, ff, bofa.c, bofb.c);

df[4] = df[3] + 1;
ff[4] = ff[3] + 1;

bodc.d[0] = 1.5;
differ_bit(szd, 5, db, df, bodc.c, bodb.c);

bofc.f[0] = 1.5;
differ_bit(szf, 5, fb, ff, bofc.c, bofb.c);

/* adjust loc(mantissa) */
df[5] -= df[6];
ff[5] -= ff[6];

/* #exponent_bits = loc(mantissa) - loc(exponent) */
df[1] = df[5] - df[4];
ff[1] = ff[5] - ff[4];

/* #mantissa_bits = #bits - #exponent_bits - 1 */
df[2] = df[0] - df[1] - 1;
ff[2] = ff[0] - ff[1] - 1;

find_exp(db, df, bodb.c);
find_exp(fb, ff, bofb.c);

/* adjust exponent */
df[7] -= df[6];
ff[7] -= ff[6];

return(0);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* MAIN - start here */

int main()
{int size[9], align[9], ssize[9], i;
char int_order[80];
int fb[40], db[40];
long ff[8], df[8];

/* these structs will help determine alignment of the primitive types */
struct cchar {
   char c;
   char x;
} cc;

struct cptr {
   char c;
   char *x;
} cp;

struct cshort {
   char c;
   short x;
} cs;

struct cint {
   char c;
   int x;
} ci;

struct clong {
   char c;
   long x;
} cl;

struct cllong {
   char c;
   long long x;
} cll;

struct cfloat {
   char c;
   float x;
} cf;

struct cdouble {
   char c;
   double x;
} cd;

/*
 * Some machines have an additional alignment for structs.
 * This struct will test for such a "feature".
 */
struct cstruct {
   struct LevOne {
      char one;
   } a;
   struct LevTwo {
      char two;
   } b;
} ct;

/*
 * This union will be used to determine the integer types
 * parameters especially the byte order.
 */
union ucsil {
   unsigned char c[32];
   short s[2];
   int i[2];
   long l[2];
} bo;

/*
 * Data type sizes are straightforward.
 */
size[0] = sizeof(char);
size[1] = sizeof(char *);
size[2] = sizeof(short);
size[3] = sizeof(int);
size[4] = sizeof(long);
size[5] = sizeof(long long);
size[6] = sizeof(float);
size[7] = sizeof(double);
size[8] = 2*sizeof(char);

ssize[0] = sizeof(cc);
ssize[1] = sizeof(cp);
ssize[2] = sizeof(cs);
ssize[3] = sizeof(ci);
ssize[4] = sizeof(cl);
ssize[5] = sizeof(cll);
ssize[6] = sizeof(cf);
ssize[7] = sizeof(cd);
ssize[8] = sizeof(ct);

printf ("/* THIS FILE IS GENERATED BY `detect' -- DO NOT EDIT */\n") ;
printf ("#ifndef _PDFORM_H\n") ;
printf ("#define _PDFORM_H\n") ;

/* first possibility: align <= size (usual case)
 * alignment is difference between struct length and member size:
 */
for (i = 0; i < 9; i++)
   align[i] = ssize[i] - size[i];

/* second possibility: align > size (e.g. Cray char)
 * alignment is half of structure size:
 */
for (i = 0; i < 9; i++)
   if (align[i] > (ssize[i] >> 1))
      align[i] = ssize[i] >> 1;

bo.i[0] = 1;
if (bo.c[0] == 1)
   strcpy(int_order, "REVERSE_ORDER");
else
   strcpy(int_order, "NORMAL_ORDER");

derive_fp_format(fb, db, ff, df);

/* print the floating point ordering info */
printf("\nint\n");
printf(" lite_int_ord_f[] = {%d", fb[0]);
for (i = 1; i < size[6]; i++)
   printf(", %d", fb[i]);
printf("}, \n");
printf(" lite_int_ord_d[] = {%d", db[0]);
for (i = 1; i < size[7]; i++)
   printf(", %d", db[i]);
printf("};\n");

/* print the floating point format info */
printf("\nlong\n");
printf(" lite_int_frm_f[] = {");
for (i = 0; i < 7; i++)
   printf("%2ldL, ", ff[i]);
printf("0x%lXL}, \n", ff[7]);
printf(" lite_int_frm_d[] = {");
for (i = 0; i < 7; i++)
   printf("%2ldL, ", df[i]);
printf("0x%lXL};\n", df[7]);

printf("\n/* Internal DATA_STANDARD */\n");
printf("data_standard\n");
printf(" lite_INT_STD = {%d, /* size of pointer */\n", 
       size[1]);
printf("            %d, %s, /* size and order of short */\n", 
       size[2], int_order);
printf("            %d, %s, /* size and order of int */\n", 
       size[3], int_order);
printf("            %d, %s, /* size and order of long */\n", 
       size[4], int_order);
printf("            %d, %s, /* size and order of long long */\n", 
       size[5], int_order);
printf("            %d, lite_int_frm_f, lite_int_ord_f, /* float definition */\n", size[6]);
printf("            %d, lite_int_frm_d, lite_int_ord_d}, /* double definition */\n", size[7]);
printf(" *lite_INT_STANDARD = &lite_INT_STD;\n");

printf("\n/* Internal DATA_ALIGNMENT */\n");
printf("data_alignment\n");
printf(" lite_INT_ALG = {%d, %d, %d, %d, %d, %d, %d, %d, %d},\n", 
       align[0], align[1], align[2], align[3], 
       align[4], align[5], align[6], align[7], align[8]);
printf(" *lite_INT_ALIGNMENT = &lite_INT_ALG;\n");

printf ("\n") ;
printf ("#endif /* !_PDFORM_H */\n") ;
return(0);}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

