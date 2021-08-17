/*****************************************************************************
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
*****************************************************************************/

#include "SiloValueView.h"
#include <SiloFile.h>
#include <QLabel>
#include <iostream>
#include <cstdlib>
using std::cerr;

// ----------------------------------------------------------------------------
//                            Value View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor:  SiloValueViewWindow::SiloValueViewWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark Miller, Tue 23 Mar 11:19:13 PDT 2004
//    Added call to free memory from the Silo DBGetVar call.
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
//    Mark C. Miller, Fri Dec  4 15:19:47 PST 2009
//    Adding support for long long type
//
//    Mark C. Miller, Mon Dec  7 09:50:19 PST 2009
//    Conditionally compile long long support only when its
//    different from long.
//
//    Mark C. Miller, Mon Jan 11 16:02:16 PST 2010
//    Made long long support UNconditionally compiled.
// ****************************************************************************
SiloValueViewWindow::SiloValueViewWindow(SiloFile *s, const QString &n, QWidget *p)
    : QMainWindow(p), silo(s), name(n)
{
    setWindowTitle(QString("Value: ")+name);

    QLabel *l = new QLabel(this);
    setCentralWidget(l);

    void *var = silo->GetVar(name);
    if (!var)
    {
        cerr << "SiloValueViewWindow::SiloValueViewWindow -- not an array var\n";
        throw;
    }

    int type = silo->GetVarType(name);
    int len  = silo->GetVarLength(name);
    char str[4096];
    switch (type)
    {
      case DB_INT:
        sprintf(str, "int: %d", *((int*)var));
        
        break;
      case DB_SHORT:
        sprintf(str, "short: %d", *((short*)var));
        break;
      case DB_LONG:
        sprintf(str, "long: %ld", *((long*)var));
        break;
      case DB_LONG_LONG:
        sprintf(str, "long long: %lld", *((long long*)var));
        break;
      case DB_FLOAT:
        sprintf(str, "float: %g", *((float*)var));
        break;
      case DB_DOUBLE:
        sprintf(str, "double: %g", *((double*)var));
        break;
      case DB_CHAR:
        if (len == 1)
            sprintf(str, "char: %c", *((char*)var));
        else
            sprintf(str, "string: %s", ((char*)var));
        break;
      case DB_NOTYPE:
        sprintf(str, "NOTYPE: ???");
        break;
      default:
        sprintf(str, "???: ???");
        break;
    }
    l->setText(str);

    free(var);
}

