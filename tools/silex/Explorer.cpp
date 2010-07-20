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

#include "Explorer.h"
#include <SiloView.h>

#include <QMenuBar>
#include <QFileDialog>
#include <QApplication>

// ****************************************************************************
//  Constructor: Explorer::Explorer
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Made it deal with failure to construct SiloView
//  
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
Explorer::Explorer(const QString &file, QWidget *p)
    : QMainWindow(p)
{
    view = new SiloView(file, this);
    if (!view->HasSiloFile())
    {
        delete view;
        view = 0;
    }
    else
    {
        setCentralWidget(view);
        setWindowTitle("Explorer: "+file);

        QMenu *filemenu = menuBar()->addMenu("&File");
        filemenu->addAction( "&Open",  this, SLOT(open()),  tr("Ctrl+O") );
        filemenu->addSeparator();
        filemenu->addAction( "E&xit", this, SLOT(close()),  tr("Ctrl+X") );
    }
}

// ****************************************************************************
//  Destructor:  Explorer::~Explorer
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 17, 2004
//
//  Modifications:
//    Mark C. Miller,Thu Jul 20 15:45:55 PDT 2006
//    Added deletion of view
// ****************************************************************************
Explorer::~Explorer()
{
    if (view) delete view;
    view = 0;
}

// ****************************************************************************
//  Method:  Explorer::open
//
//  Purpose:
//    Open a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon May 17 11:50:08 PDT 2004
//    Change the window caption when opening a new file.
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
Explorer::open()
{
    QString file = 
        QFileDialog::getOpenFileName(this,"Open file...","",
                                     "Silo files (*.silo *.root *.pdb);;"
                                     "All files (*)");
    if (file.isNull())
        return;

    setWindowTitle("Explorer: "+file);
    view->Set(file);
}
