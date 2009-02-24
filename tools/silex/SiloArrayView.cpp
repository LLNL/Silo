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

#include "SiloArrayView.h"
#include <SiloFile.h>
#include <QListWidget>
#include <iostream>
#include <cstdlib>
using std::cerr;

// ----------------------------------------------------------------------------
//                            Array View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloArrayViewWindow::SiloArrayViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark Miller, Tue 23 Mar 11:19:13 PDT 2004
//    Fixed problem with character arrays longer than 1024 chars
//    Added missing call to free memory allocated by GetVar call
//
//    Jeremy Meredith, Mon May 17 11:56:24 PDT 2004
//    Rewrote character array code to fix garbage/missing character output
//    and prevent it splitting at 1024 bytes.
//
//    Jeremy Meredith, Wed Oct 13 20:32:56 PDT 2004
//    Split on spaces, semicolons, or by length depending on a heuristic
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloArrayViewWindow::SiloArrayViewWindow(SiloFile *s, const QString &n,
                                         QWidget *p)
    : QMainWindow(p), silo(s), name(n)
{
    setWindowTitle(QString("Array: ")+name);

    lb = new QListWidget(this);
    setCentralWidget(lb);

    void *var = silo->GetVar(name);
    if (!var)
    {
        cerr << "SiloArrayViewWindow::SiloArrayViewWindow -- not an array var\n";
        throw;
    }

    int type = silo->GetVarType(name);
    int len  = silo->GetVarLength(name);

    if (type != DB_CHAR)
    {
        // Just a bunch of numbers: one per line
        for (int i=0; i<len; i++)
        {
            char str[256];
            switch (type)
            {
              case DB_INT:
                sprintf(str, "%-4d: %d", i, ((int*)var)[i]);
                break;
              case DB_SHORT:
                sprintf(str, "%-4d: %d", i, ((short*)var)[i]);
                break;
              case DB_LONG:
                sprintf(str, "%-4d: %ld", i, ((long*)var)[i]);
                break;
              case DB_FLOAT:
                sprintf(str, "%-4d: %g", i, ((float*)var)[i]);
                break;
              case DB_DOUBLE:
                sprintf(str, "%-4d: %g", i, ((double*)var)[i]);
                break;
              case DB_CHAR:
                sprintf(str, "%-4d: %c", i, ((char*)var)[i]);
                break;
              case DB_NOTYPE:
                sprintf(str, "%-4d: NOTYPE", i);
                break;
              default:
                sprintf(str, "%-4d: type ???", i);
                break;
            }
            lb->addItem(str);
        }
    }
    else
    {
        // Guess how to split the string based on a heuristic:
        //  - If the length is short, just stick it in
        //  - If there are an appropriate number of semicolons or spaces, 
        //                                      split on them
        //  - Otherwise, split by a constant length
        int numSemi = 0;
        int numSpace = 0;
        int numIdeal = (len/100) + 1;
        int numMax   = (len/400) + 1;
        for (int j=0; j<len; j++)
        {
            if (((char*)var)[j] == ' ')
                numSpace++;
            if (((char*)var)[j] == ';')
                numSemi++;
        }

        if (numSpace < numMax && numSemi < numMax)
        {
            // Copy the character array into a temporary buffer,
            // splitting by length
            char *str = new char[len+1];
            char *p = str;
            int tmplen = 0;
            for (int i=0; i<len; i++)
            {
                *p = ((char*)var)[i];
                if (tmplen == 400)
                {
                    p++;
                    *p = '\0';
                    lb->addItem(str);
                    p = str;
                    tmplen = 0;
                }
                else
                {
                    p++;
                    tmplen++;
                }
            }
            if (p != str)
            {
                *p = '\0';
                lb->addItem(str);
            }
            delete[] str;
        }
        else
        {
            // Copy the character array into a temporary buffer,
            // splitting at the separator, and putting the result in a list box
            char splitChar;
            if (numSpace==0)
                splitChar = ';';
            else if (numSemi==0)
                splitChar = ' ';
            else if (abs(numSpace-numIdeal) > abs(numSemi-numIdeal))
                splitChar = ';';
            else
                splitChar = ' ';
            
            char *str = new char[len+1];
            char *p = str;
            for (int i=0; i<len; i++)
            {
                *p = ((char*)var)[i];
                if (*p == splitChar)
                {
                    if (p != str)
                    {
                        *p = '\0';
                        lb->addItem(str);
                    }
                    p = str;
                }
                else
                {
                    p++;
                }
            }
            if (p != str)
            {
                *p = '\0';
                lb->addItem(str);
            }
            delete[] str;
        }
    }

    free(var);
}