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

#include "SiloView.h"
#include <SiloFile.h>
#include <SiloDirTreeView.h>
#include <SiloDirView.h>
#include <SiloValueView.h>
#include <SiloArrayView.h>
#include <SiloObjectView.h>
#include <QHeaderView>
#include <QMessageBox>
#include <iostream>

// ----------------------------------------------------------------------------
//                            Silo View
// ----------------------------------------------------------------------------

// ****************************************************************************
//  Constructor: SiloView::SiloView
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Sep 24 11:19:27 PDT 2003
//    I added a call to set the root directory in the directory-contents
//    panel.  This was unnecessary in Qt versions before 3.x.
//
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Made it more graceful on failure to open silo
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
SiloView::SiloView(const QString &file, QWidget *p)
    : QSplitter(p)
{
    silo        = new SiloFile(file);
    if (!silo->IsOpen())
    {
        delete silo;
        silo = 0;
        dirTreeView = 0;
        dirView = 0;
    }
    else
    {
        dirTreeView = new SiloDirTreeView(silo, this);
        dirView     = new SiloDirView(this);

        connect(dirTreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                this,        SLOT(SetDir(QTreeWidgetItem*,QTreeWidgetItem*)));
        connect(dirView,     SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
                this,        SLOT(ShowItem(QTreeWidgetItem*,int)));

        dirTreeView->OpenRootDir();
        dirView->Set(silo->root);

        dirTreeView->header()->setResizeMode(QHeaderView::Interactive);
        dirView->header()->setResizeMode(QHeaderView::Interactive);
    }
}

// ****************************************************************************
//  Destructor:  SiloView::~SiloView
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 17, 2004
//
//  Modifications:
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Added some missing delete calls
//
// ****************************************************************************
SiloView::~SiloView()
{
    if (silo) delete silo;
    if (dirTreeView) delete dirTreeView;
    if (dirView) delete dirView;
}

// ****************************************************************************
//  Method:  SiloView::Set
//
//  Purpose:
//    Replace with a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 12 20:53:28 PDT 2004
//    Explicitly set the dirView directory.  Newer Qt versions don't seem to 
//    do it automatically by setting the first child (root dir) as selcted
//    inside SiloDirTreeView.
//
//    Mark C. Miller, Thu Jul 20 15:45:55 PDT 2006
//    Made it more graceful if Silo failed to open
//
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloView::Set(const QString &file)
{
    delete silo;
    silo = new SiloFile(file);
    if (!silo->IsOpen())
    {
        dirTreeView = 0;
        dirView = 0;
    }
    else
    {
        dirTreeView->Set(silo);
        dirTreeView->OpenRootDir();
        dirView->ChangeDir(dirTreeView->topLevelItem(0));
    }
}

// ****************************************************************************
//  Method:  SiloView::SetDir
//
//  Purpose:
//    Tell the current SiloDirView directories.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloView::SetDir(QTreeWidgetItem *i, QTreeWidgetItem*)
{
    if (i)
        dirView->ChangeDir(i);
}

// ****************************************************************************
//  Method:  SiloView::ShowVariable
//
//  Purpose:
//    Pop up a new window to show a variable or simple array.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::ShowVariable(const QString &name)
{
    int len = silo->GetVarLength(name);
    if (len == 1)
    {
        SiloValueViewWindow *vv = new SiloValueViewWindow(silo, name, this);
        vv->show();
    }
    else
    {
        SiloArrayViewWindow *av = new SiloArrayViewWindow(silo, name, this);
        av->show();
        av->resize(av->sizeHint());
    }
}

// ****************************************************************************
//  Method:  SiloView::ShowObject
//
//  Purpose:
//    Pop up a new window to show a compound object.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
void
SiloView::ShowObject(const QString &name)
{
    SiloObjectViewWindow *ov = new SiloObjectViewWindow(silo, name, this);
    connect(ov,   SIGNAL(showRequested(const QString&)),
            this, SLOT(ShowUnknown(const QString&)));
    ov->show();
}

// ****************************************************************************
//  Method:  SiloView::ShowUnknown
//
//  Purpose:
//    Pop up a new window to show an item based on its type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed May 11 12:42:12 PDT 2005
//    Show an error message if we get an invalid object.
//
// ****************************************************************************
void
SiloView::ShowUnknown(const QString &name)
{
    DBObjectType type = silo->InqVarType(name);

    if (type == DB_INVALID_OBJECT)
    {
        QMessageBox::warning(this, "silex",
           "Could not determine the type of this object.\n"
           "The file may have been written using an incomplete driver.", "OK");
        return;
    }


    bool isObject = (type != DB_VARIABLE);

    if (isObject)
        ShowObject(name);
    else
        ShowVariable(name);
}

// ****************************************************************************
//  Method:  SiloView::ShowItem
//
//  Purpose:
//    Wrapper for ShowUnknown appropriate for a QTreeWidgetItem callback.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 20 17:28:45 EST 2008
//    Ported to Qt4.
//
// ****************************************************************************
void
SiloView::ShowItem(QTreeWidgetItem *i, int)
{
    SiloDirViewItem *item = (SiloDirViewItem*)i;
    if (!item->dir)
        return;

    QString name = item->dir->path;
    if (name != "/")
        name += "/";
    name += item->name;

    ShowUnknown(name);
}
