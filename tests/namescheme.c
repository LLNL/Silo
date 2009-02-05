/*****************************************************************************
*
* Copyright (c) 2000 - 2009, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <silo.h>
#include <string.h>

int main()
{
    int i;
    int P[100], U[4];

    /* Test a somewhat complex expression */ 
    DBnamescheme *ns = DBMakeNamescheme("@foo_%+03d@3-((n % 3)*(4+1)+1/2)+1");
    if (strcmp(DBGetName(ns, 25), "foo_+01") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test ?:: operator */
    ns = DBMakeNamescheme("@foo_%d@(n-5)?14:77:");
    if (strcmp(DBGetName(ns, 6), "foo_14") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test multiple conversion specifiers */
    ns = DBMakeNamescheme("|foo_%03dx%03d|n/5|n%5");
    if (strcmp(DBGetName(ns, 17), "foo_003x002") != 0)
       return 1;
    if (strcmp(DBGetName(ns, 20), "foo_004x000") != 0)
       return 1;
    if (strcmp(DBGetName(ns, 3), "foo_000x003") != 0)
       return 1;
    DBFreeNamescheme(ns);

    /* Test embedded string value results */
    ns = DBMakeNamescheme("#foo_%s#(n-5)?'master':'slave':");
    if (strcmp(DBGetName(ns, 6), "foo_master") != 0)
        return 1;
    DBFreeNamescheme(ns);

    /* Test array-based references in a name scheme */
    for (i = 0; i < 100; i++)
        P[i] = i*5;
    for (i = 0; i < 4; i++)
        U[i] = i*i;
    ns = DBMakeNamescheme("#foo_%03dx%03d#$P[n]#$U[n%4]", P, U);
    if (strcmp(DBGetName(ns, 17), "foo_085x001") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 18), "foo_090x004") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 19), "foo_095x009") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 20), "foo_100x000") != 0)
        return 1;
    if (strcmp(DBGetName(ns, 21), "foo_105x001") != 0)
        return 1;
    DBFreeNamescheme(ns);

    return 0;
}
